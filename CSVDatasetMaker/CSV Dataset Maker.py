#!/usr/bin/env python
# coding: utf-8

import glob
import os
import csv
import copy
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D

sns.set_style("darkgrid")
fig = plt.figure()

#実行ディレクトリを取得
dir_csv_viz = "C:\\Users\\administrator\\CSVDatasetMaker"
#入力ディレクトリ名を指定
dir_input = dir_csv_viz + "\\input"
#出力ディレクトリ名を指定
dir_output =dir_csv_viz + "\\output"

#print(os.getcwd())
print("Current Directory is : " + dir_csv_viz)
print("Input Directory is : " + dir_input)
print("Output Directory is : " + dir_output)

#取得するCSVファイルの名前規則設定ファイルを読み込み
fp_ftype_conf = open(dir_csv_viz + "\\configFileType.txt", "r")
file_type = ""
for line in fp_ftype_conf:
    file_type = line
fp_ftype_conf.close()
print(file_type)

#データセット用にリネームするかどうか決める(しない場合はそのまま)
rename = input("rename?[no = 0, from configfile = 1, or write by yourself]: ")

#入力ディレクトリ内CSVファイルの名前リストを取得
csv_name_list = glob.glob(dir_input + "\\" + file_type)
#print(csv_name_list)

#CSVファイル名リストのファイルを1つずつ再構成していく
serial_num = 0
for f in csv_name_list:
    fp_csv = open(f, "r")
    
    fname = ""
    #出力ファイル名の生成
    if rename == '0':
        fname = str(copy.copy(f))
        fname = fname.lstrip(dir_input)
        fname = fname.replace(".csv","")
        fname = fname + "_" + str(serial_num)
        
    elif rename == '1':
        #リネームに関する設定ファイルを読み込み
        fp_rename_conf = open(dir_csv_viz + "\\configRename.txt", "r")
        rename = ""
        for line in fp_rename_conf:
            rename = line
        fname = rename + "_" + str(serial_num)
        fp_rename_conf.close()
    else:
        fname = rename + "_" + str(serial_num)

    fpath = dir_output + "\\dataset_" + fname + ".csv"
    #print(fpath)
    fp_out = open(fpath, "w")
    
    #1つのCSVからデータを読み取る
    reader = csv.reader(fp_csv)
    data = [row for row in reader]
    data_T = [list(t) for t in zip(*data)]
    #print(data)
    #print(data_T)
    
    #id,x,y,confidency,zそれぞれの値列として取得
    ids = [int(id) for id in data_T[0]]
    x_values = [int(x) for x in data_T[1]]
    #y_values = [-1 * int(y) for y in data_T[2]]
    y_values = [int(y) for y in data_T[2]]
    confidences = [float(c) for c in data_T[3]]
    z_values = [int(z) for z in data_T[4]]
    #print(ids)
    #print(x_values)
    #print(y_values)
    #print(confidences)
    #print(z_values)
    
    #x,y,zのみを関節点ID0～45まで順番に並べたCSVを生成する
    writer = csv.writer(fp_out)
    buffer = []
    for i in range(len(x_values)):
        buffer.append(x_values[i])
        buffer.append(y_values[i])
        buffer.append(z_values[i])
    buffer.append(serial_num)
    #print(buffer)
    writer.writerow(buffer)

    fp_csv.close()
    fp_out.close()
    serial_num += 1
print("completed saving !")
