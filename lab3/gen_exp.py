import os
import sys
import glob


input_dir = sys.argv[1] if len(sys.argv) > 1 else 'data'

input_path_pattern = f'{input_dir}/*.tsp'

data_file = f'report/data/{input_dir}.csv'
exp_data_file = f'report/data/exp_{input_dir}.csv'
cycle_file = lambda input_file: f'report/cycle/{os.path.basename(input_file)[:-4]}.csv'


# init
with open(data_file, 'w') as file:
    file.write('input_data,num_vertices,avg_cycle_weight,min_cycle_weight\n')


# run
def sort_key(file_name):
    base_name = os.path.basename(file_name)
    return int(base_name.split('.')[0][3:])

files = glob.glob(input_path_pattern)
files = sorted(files, key=sort_key)

for file_path in files:
    print(
        f'./tsp -f {file_path} -s {data_file} -b {cycle_file(file_path)}'
        ' -mi 50 -spc -ps 100 -mp 0.2'
    )
