# coding: utf-8
import pandas as pd
import datetime
import numpy as np

# Algo.
def PositionCorrelation(position_pd, camera_loc, obDe_df, obDe_all_split_index):
    PC_results = {}

    for times, taken_event in enumerate(obDe_all_split_index):
        PC_result = []
        what_list = []
        whatsets = obDe_df.loc[taken_event[0]:taken_event[-1], ['what']].values
        for whatset in whatsets:
            whats = whatset[0].split(',')
            for what in whats:
                if what not in what_list:
                    what_list.append(what)
        for what in what_list:


            what_detected_times_df = obDe_df.loc[obDe_df['what'].str.contains(what), ['timestamp']].reset_index(drop=True)
            what_detected_times_df['timestamp'] = pd.to_datetime(what_detected_times_df['timestamp'], format='%H:%M:%S:%f')
            ob_start_time = what_detected_times_df.iloc[0]['timestamp']
            ob_end_time = what_detected_times_df.iloc[what_detected_times_df.shape[0]-1]['timestamp']

            pos_duringET = position_pd.loc[(position_pd['timestamp'] >= ob_start_time) & (position_pd['timestamp'] <= ob_end_time), 
                                           [("a%d" % x) for x in range(position_pd.shape[1]-1)]]

            pos_duringET = pos_duringET.reset_index(drop=True)
            pos_inTouch = pos_duringET.iloc[pos_duringET.shape[0]//2,]

            num_ids = pos_inTouch.shape[0] // 6
            for i in range(num_ids):
                name = pos_inTouch[i * 6]
                rightWristX = pos_inTouch[i * 6 + 3]
                rightWristY = pos_inTouch[i * 6 + 4]
                rightWristZ = pos_inTouch[i * 6 + 5]

                score = 1/(((camera_loc[0] - rightWristX)**2 + (camera_loc[1] - rightWristY)**2 + (camera_loc[2] - rightWristZ)**2)**1/2)

                PC_result.append([name, what, score].copy())
        PC_results[times] = PC_result
    return PC_results