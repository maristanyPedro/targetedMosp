from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from scipy.stats.mstats import gmean
plt.rcParams['text.usetex'] = True

TIME_LIMIT = 18000

#Only the instances solved matching the conditions defined in this function are regarded as solved!
def define_solvability(solTime):
    if pd.isnull(solTime) or solTime > TIME_LIMIT:
        return False
    return True

def mergeInstancesAndResults(instances, results):
    instances.sort_values(by=["SOURCE", "TARGET"], inplace=True, ignore_index = True)

    results_by_algorithms = results.groupby(by="ALGO", sort=False)
    master_df = instances
    for algo, group_results in results_by_algorithms:
        group_results = group_results.reset_index(drop=True)
        group_results = group_results.add_suffix(".{}".format(algo))
        group_results = group_results.rename(columns={"SOURCE.{}".format(algo):"SOURCE", "TARGET.{}".format(algo):"TARGET", "GRAPH.{}".format(algo):"GRAPH"})
        master_df = master_df.merge(right=group_results, how='left', on=['SOURCE', 'TARGET'])
        master_df["ALGO.{}".format(algo)] = algo
        #Generate Boolean column that indicates whether an instance was solved by the considered algorithm!
        master_df["SOLVABILITY.{}".format(algo)] = master_df.apply(lambda x: define_solvability(x["TIME.{}".format(algo)]), axis=1)
    return master_df

def geoMeanWithoutNans(df, columnName):
    #print("{} CURRENTLY CONSIDERING: ".format(columnName))
    #print(df[~df[columnName].isnull()][columnNam   e].to_string())
    #print("{} SUGGESTION: ".format(columnName))
    #print(df[df[columnName]<18000][columnName].to_string())
    column = df[~df[columnName].isnull()][columnName]
    if "TIME" in columnName:
        column = df[df[columnName]<TIME_LIMIT][columnName]
    #return gmean(column)
    return column.mean()
####################################################################

results_columns=["ALGO", "GRAPH", "SOURCE", "TARGET", "EXTRACTED", "PERMANENT", "AT_TARGET", "TIME", "MEMORY", "MAX_HEAP"]
#solutionsFileName = "namoa_lazy_tmda_road.csv"
solutionsFileName = "temp.csv"
filename = solutionsFileName.split('.')[0]
solutions_df = pd.read_csv(solutionsFileName, sep=';', header=None, names=results_columns)
solutions_df["GRAPH"] = solutions_df["GRAPH"].apply(lambda x: Path(x).stem)

algos = [a for a, _ in solutions_df.groupby(by="ALGO", sort=False)]
for i in range(len(algos)):
    print("{}: {}".format(i, algos[i]))
algoIndex = int(input("What's the index of the algorithm you want to use as the reference algorithm when calculating speedups?"))
referenceAlgo = algos[algoIndex]

print("Reference algorithm set to: {}.".format(referenceAlgo))

for roadGraphName, results_df in solutions_df.groupby(by="GRAPH", sort=False):
    instance_df = pd.read_csv("{}.inst".format(roadGraphName), sep=' ', header=None, names=["GRAPH", "SOURCE", "TARGET"])
    instance_df["GRAPH"] = instance_df["GRAPH"].apply(lambda x: Path(x).stem)

    # font = {'family' : 'normal',
    #         'size'   : 12}
    #
    # matplotlib.rc('font', **font)

    c1 = '#FFC857'
    c2 = '#255F85'

    total_df = mergeInstancesAndResults(instance_df, results_df)
    total_df["SPEEDUP"] = total_df["TIME.NAMOA_LAZY"] / total_df["TIME.T-MDA"]
    print(total_df["SPEEDUP"])
    #print(total_df.to_string())


    ax1 = total_df.plot(kind='scatter', x="AT_TARGET.T-MDA", y='TIME.T-MDA',
             label='T-MDA', color=c1)
    ax2 = total_df.plot(kind='scatter', x="AT_TARGET.NAMOA_LAZY", y='TIME.NAMOA_LAZY',
             label='NAMO$A^*_{dr}$-lazy', color=c2, ax=ax1)
    ax1.set_yscale('log')
    ax1.yaxis.grid(True)
    ax1.set_xlabel("Efficient $s$-$t$-paths")
    ax1.set_ylabel("Duration [s]")
    #ax1.set_yscale('log')
    ax1.set_title("One-to-One 3-dim. MOSP instances on {} road network".format(roadGraphName))

    plt.savefig("{}-{}.pdf".format(filename, roadGraphName), format="pdf")
    #plt.savefig("{}-{}.png".format(filename, roadGraphName), dpi=300, bbox_inches='tight')

    graphs = [g for g, _ in total_df.groupby(by="GRAPH")]
    graphName = graphs[0]

    solved_instances_only = {}
    speedup_by_instance = {}
    for algo in algos:
        total_df.loc[total_df["SOLVABILITY.{}".format(algo)]==False, "TIME.{}".format(algo)] = TIME_LIMIT
        solved_instances_only[algo] = total_df.loc[total_df["SOLVABILITY.{}".format(algo)]]
        if algo == referenceAlgo:
            continue
        speedup_by_instance[algo] = total_df["TIME.{}".format(algo)]/total_df["TIME.{}".format(referenceAlgo)]

    #print("Overall max: {}. Sliced max: {}".format(total_df["TIME.T-MDA"].max(), solved_instances_only["T-MDA"]["TIME.T-MDA"].max()))

#    print("\\multirow{{3}}{{*}}{{{}}} & Min. & {:.0f} & {:.0f} & {:.2f} & {:.0f} & {:.2f} & {:.2f}\\\\".format(graphName,
#                                                             solved_instances_only["T-MDA"]["AT_TARGET.T-MDA"].min(),
#                                                             solved_instances_only["T-MDA"]["EXTRACTED.T-MDA"].min(),
#                                                             solved_instances_only["T-MDA"]["TIME.T-MDA"].min(),
#                                                             solved_instances_only["NAMOA_LAZY"]["EXTRACTED.NAMOA_LAZY"].min(),
#                                                             solved_instances_only["NAMOA_LAZY"]["TIME.NAMOA_LAZY"].min(),
#                                                                speedup_by_instance["NAMOA_LAZY"].min()))
#    print("& Max. & {:.0f} & {:.0f} & {:.2f} & {:.0f} & {:.2f} & {:.2f}\\\\".format(total_df["AT_TARGET.T-MDA"].max(),
#                                                             solved_instances_only["T-MDA"]["EXTRACTED.T-MDA"].max(),
#                                                             solved_instances_only["T-MDA"]["TIME.T-MDA"].max(),
#                                                             solved_instances_only["NAMOA_LAZY"]["EXTRACTED.NAMOA_LAZY"].max(),
#                                                             solved_instances_only["NAMOA_LAZY"]["TIME.NAMOA_LAZY"].max(),
#                                                             speedup_by_instance["NAMOA_LAZY"].max()))
#    print("& Avg. & {:.0f} & {:.0f} & {:.2f} & {:.0f} & {:.2f} & {:.2f}\\\\".format(gmean(total_df["AT_TARGET.T-MDA"]),
#                                                             gmean(total_df["EXTRACTED.T-MDA"]),
#                                                             gmean(total_df["TIME.T-MDA"]),
#                                                             gmean(total_df["PERMANENT.NAMOA_LAZY"]),
#                                                             gmean(total_df["TIME.NAMOA_LAZY"]),
#                                                             gmean(speedup_by_instance["NAMOA_LAZY"])))

    slicesBounds = [0, 100, 1000, 5000, 10000, 1000000]
    for i in range(len(slicesBounds)-1):
        #Take only the instances that the reference algorithm solved within the specified time range.
        sliced_df = total_df.loc[(total_df['AT_TARGET.T-MDA']>slicesBounds[i]) & (total_df['AT_TARGET.T-MDA']<=slicesBounds[i+1]) & (total_df['SOLVABILITY.{}'.format(referenceAlgo)])]
        #print(sliced_df.to_string())
        
        speedup_by_instance = {}
        toBePrinted = ""
        if i == 0:
            toBePrinted += "\\multirow{{5}}{{*}}{{{}}} & ({}, {}] & {:.0f} & ".format(\
                graphName, slicesBounds[i], slicesBounds[i+1], geoMeanWithoutNans(sliced_df, "AT_TARGET.T-MDA"))
        else:
            toBePrinted += "& ({}, {}] & {:.0f} & ".format(\
                slicesBounds[i], slicesBounds[i+1], geoMeanWithoutNans(sliced_df, "AT_TARGET.T-MDA"))
        
        for algo in algos:
            toBePrinted += "{}/{} & {:.0f} & {:.2f} & ".format(\
            len(sliced_df.loc[total_df["SOLVABILITY.{}".format(algo)]==True]), len(sliced_df), \
                geoMeanWithoutNans(sliced_df, "EXTRACTED.{}".format(algo)),
                geoMeanWithoutNans(sliced_df, "TIME.{}".format(algo)))
            if algo == referenceAlgo:
                continue
            else:
                speedup_by_instance = sliced_df["TIME.{}".format(algo)]/sliced_df["TIME.{}".format(referenceAlgo)]
                # toBePrinted += "{:.2f} \\\\".format(gmean(speedup_by_instance))
                toBePrinted += "{:.2f} \\\\".format(speedup_by_instance.mean())
        print(toBePrinted)
        
    print("\\midrule")
    #print("RESULT: \n")

###################################NEW########################################
#        lineStart = ""
#        if i == 0:
#            lineStart += "\\multirow{{5}}{{*}}{{{}}} &".format(graphName)
#        else:
#            lineStart += "& "
#        toBePrinted += lineStart + "({}, {}] & {:.0f} & {:.0f} & ".format(\
#            slicesBounds[i], slicesBounds[i+1], len(sliced_df), geoMeanWithoutNans(sliced_df, "AT_TARGET.T-MDA"))
#
#        for algo in algos:
#            toBePrinted += "{:.0f} & {:.2f} & ".format(\
#                geoMeanWithoutNans(sliced_df, "EXTRACTED.{}".format(algo)),
#                geoMeanWithoutNans(sliced_df, "TIME.{}".format(algo)))
#            if algo == referenceAlgo:
#                continue
#            else:
#                speedup_by_instance = sliced_df["TIME.{}".format(algo)]/sliced_df["TIME.{}".format(referenceAlgo)]
#                toBePrinted += "{:.2f} \\\\".format(gmean(speedup_by_instance))
#        print(toBePrinted)
#
#    print("\\midrule")

