
# coding: utf-8

import pandas as pd
import time
import datetime

# Algo.
def ReachingAndTaken(ro_namedfs, obDe_df, obDe_all_split_index):
    taken_event_results = {}
    for times, taken_event in enumerate(obDe_all_split_index):
        taken_event_result = []
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
            start_time = what_detected_times_df.iloc[0]['timestamp']
            end_time = what_detected_times_df.iloc[what_detected_times_df.shape[0]-1]['timestamp']

            for name, ro_df in ro_namedfs.items():
                range_HEinTE = ro_df.loc[(ro_df['timestamp'] >= start_time.to_datetime64()) & (ro_df['timestamp'] <= end_time.to_datetime64()), ['yn']]
                numerator = range_HEinTE.sum().values[0]
                denominator = range_HEinTE.shape[0]
                begin_in = range_HEinTE.index[0]
                end_in = range_HEinTE.index[-1]
                if ro_df.iloc[begin_in]['yn'] == 1:
                    countdown = 1
                    while True:
                        if ro_df.iloc[begin_in-countdown]['yn'] == 1:
                            denominator = denominator+1
                            countdown = countdown+1
                        else:
                            break
                if ro_df.iloc[end_in]['yn'] == 1:
                    countdown = 1
                    while True:
                        if ro_df.iloc[end_in+countdown]['yn'] == 1:
                            denominator = denominator+1
                            countdown = countdown+1
                        else:
                            break

                score = numerator/denominator

                taken_event_result.append([name, what, score].copy())

        taken_event_results[times] = taken_event_result
    return taken_event_results
