//
// Created by bzfmaris on 02.02.22.
//

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <array>
#include <cmath>
#include <memory>
#include <vector>

constexpr std::size_t ROW_LENGTH = 200;
//constexpr std::size_t ROW_LENGTH = 60000;

template <typename Data>
struct Pool {
    typedef std::array<Data, ROW_LENGTH> Row;
    std::vector<std::unique_ptr<Row>> rows;
    Data* firstFreeSpace;

    Pool():
            rows(1),
            firstFreeSpace{nullptr} {
        rows[0] = std::move(std::make_unique<Row>());
        link(0);
    }

    ~Pool() = default;

    /**
     * Access the row in this->rows[rowId]. This method is called when this row is created. The
     * elements in it need to be inserted into the list of free available labels.
     * @param rowId The position in this->rows of the row to be modified.
     */
    inline void link(size_t rowId) {
        assert(this->firstFreeSpace == nullptr);
        Row& row{*this->rows[rowId]};
        this->firstFreeSpace = &row.front();
        for (size_t i = 0; i < ROW_LENGTH - 1; ++i) {
            row[i].next = &row[i+1];
        }
        row.back().next = nullptr;
    }

    inline Data* getFirstFree() {
        Data* d = this->firstFreeSpace;
        this->firstFreeSpace = this->firstFreeSpace->next;
        return d;
    }

    Data* newItem() {
        if (this->firstFreeSpace != nullptr) {
            return getFirstFree();
        }
        else {
            this->rows.push_back(std::move(std::make_unique<Row>()));
            this->link(this->rows.size()-1);
            return getFirstFree();
        }
    }

    inline void free(Data* p) {
        p->next = this->firstFreeSpace;
        this->firstFreeSpace = p;
    }

    size_t size() const {
        return this->rows.size()*ROW_LENGTH;
    }
};

#endif //MEMORYPOOL_H
