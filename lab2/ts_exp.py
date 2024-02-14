import os
import sys
import glob
import pandas as pd
import subprocess
from itertools import product
import random



tsp_mode = 'ts'
input_path_pattern = f'data/*.tsp'
data_file = f'report/data/exp_{tsp_mode}.csv'
opt_file = f'report/data/optimal.csv'

data_set = lambda file_name: os.path.basename(file_name).split('.')[0]

params = {
    'max_taboo_list_size': {10, 20, 50, 100},
    'max_iterations': {5, 10, 20, 50},
}


# run the experiments
run_tsp = sys.argv[1] if len(sys.argv) > 1 else False
if run_tsp:
    # init the result file
    with open(data_file, 'w') as file:
        file.write(
            f"input_data,num_vertices,avg_cycle_weight,min_cycle_weight,{','.join(params.keys())}\n"
        )

    # generate the test file paths
    def sort_key(file_name):
        return int(data_set(file_name)[3:])

    files = glob.glob(input_path_pattern)
    files = sorted(files, key=sort_key)


    # run the experiment for all parameter combinations
    keys, values = zip(*params.items())
    combinations = [dict(zip(keys, p)) for p in product(*values)]

    for comb in combinations:
        for file_path in files:
            cmd = (
                f"./tsp -f {file_path} -m {tsp_mode} -s {data_file} -e"
                f" -tls {comb['max_taboo_list_size']}"
                f" -tmi {comb['max_iterations']}"
            )
            print(cmd)
            process = subprocess.run(cmd.split())
            print('\n\n')
        print('\n')


# read the experiment output file
output_df = pd.read_csv(data_file)
output_df.drop('num_vertices', axis=1, inplace=True)
output_df.input_data = output_df.input_data.apply(data_set)

# read the optimum cycle weight file
opt_df = pd.read_csv(opt_file)


# generate the results
result_df = pd.merge(output_df, opt_df, on='input_data', how='left')
result_df['min_cycle_weight_err'] = (
    abs(result_df.min_cycle_weight - result_df.opt_cycle_weight) / result_df.opt_cycle_weight
)
result_df['avg_cycle_weight_err'] = (
    abs(result_df.avg_cycle_weight - result_df.opt_cycle_weight) / result_df.opt_cycle_weight
)

# extract the lowest average and lowest minimum average error
param_cols = list(params.keys())

min_grouped = result_df.groupby(param_cols).min_cycle_weight_err.min()
avg_grouped = result_df.groupby(param_cols).avg_cycle_weight_err.min()

lowest_min_cycle_weight_err = result_df[
    result_df.groupby(param_cols).min_cycle_weight_err.transform('min') == min_grouped.min()
]
lowest_avg_cycle_weight_err = result_df[
    result_df.groupby(param_cols).avg_cycle_weight_err.transform('min') == avg_grouped.min()
]

result_cols = (
    ['input_data'] + param_cols + ['min_cycle_weight', 'avg_cycle_weight', 'opt_cycle_weight']
)

print("Combinations with the lowest minimum cycle weight error:")
print(lowest_min_cycle_weight_err[result_cols + ['min_cycle_weight_err']])

print("\n\nCombination with the lowest average cycle weight error:")
print(lowest_avg_cycle_weight_err[result_cols + ['avg_cycle_weight_err']])
