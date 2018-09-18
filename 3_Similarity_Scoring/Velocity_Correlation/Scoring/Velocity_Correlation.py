# coding: utf-8
import pandas as pd
import datetime
import numpy as np
from dtw import dtw
from scipy.signal import savgol_filter
import matplotlib.pyplot as plt
from matplotlib import rc

rc('mathtext', default='regular')

# Algo.
def VelocityCorrelation_earx(ro_raw_namedfs, obDe_df, obDe_all_split_index, camera_dire_forEarX):
    VC_results = {}
    for times, taken_event in enumerate(obDe_all_split_index):
        VC_result = []
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

            delta = (ob_end_time - ob_start_time).total_seconds()
            delta = datetime.timedelta(seconds=delta)
    #         windows_quarter_delta = datetime.timedelta(seconds=config['windows_size']/2)
            ro_start_time = ob_start_time - delta
            ro_end_time = ob_end_time + delta

            for name, ro_df in ro_raw_namedfs.items():
                earx_time = ro_df.loc[(ro_df['timestamp'] >= ro_start_time) & (ro_df['timestamp'] <= ro_end_time) , ['earx', 'timestamp']]
                earx_time = earx_time.reset_index(drop=True)

                acc = (earx_time['earx']*camera_dire_forEarX).values
                acc = savgol_filter(acc, 31, 4)
                time = earx_time['timestamp']
                #Suppose intial velocity
                v = [0]    
                for j in range(0, len(acc)-1):
                    v.append(acc[j]*((time[j+1]-time[j]).total_seconds()) + v[j])


                x = np.linspace(0, 2*np.pi, len(v))
                y = np.sin(x)
                simV = y
                dy = np.zeros(y.shape, np.float)
                dy[0:-1] = np.diff(y)/np.diff(x)
                dy[-1] = (y[-1]-y[-2])/(x[-1]-x[-2])
                simAcc = dy
                
                #plot image
                plt.plot(acc, label='Acc')
                plt.plot(v, label='Velocity')
                plt.plot(simV, label='Simulated Velocity')
#                 plt.plot(simAcc, label='Simulated Acc')
                plt.legend(loc='best')
                plt.title(name)
                plt.show() 

                acc_rs = acc.reshape(-1, 1)
                v_rs = np.array(v).reshape(-1, 1)
                simAcc_rs = np.array(simAcc).reshape(-1, 1)
                simV_rs = np.array(simV).reshape(-1, 1)
#                 dist_norm2 = np.linalg.norm(acc_rs - simAcc_rs, ord=None)
#                 dist_dtw, cost, acc, path = dtw(acc_rs, simAcc_rs, dist=lambda acc_rs, simAcc_rs: np.linalg.norm(acc_rs - simAcc_rs, ord=None))
#                 dist_norm2 = np.linalg.norm(v_rs - simV_rs, ord=None)
#                 dist_dtw, cost, acc, path = dtw(v_rs, simV_rs, dist=lambda v_rs, simV_rs: np.linalg.norm(v_rs - simV_rs, ord=None))
                score = np.correlate(v,simV)
                print('Correlation:', score)
#                 score = 1/dist_norm2
#                 print('Norm2 Distance:', dist_norm2)
#                 print('DTW Distance:', dist_dtw)
#                 print('Score:', score)
                VC_result.append([name, what, score[0]].copy())
        VC_results[times] = VC_result
    return VC_results


# Algo.
def VelocityCorrelation_eary(ro_raw_namedfs, obDe_df, obDe_all_split_index, camera_dire_forEarY):
    VC_results = {}
    for times, taken_event in enumerate(obDe_all_split_index):
        VC_result = []
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

            delta = (ob_end_time - ob_start_time).total_seconds()
            delta = datetime.timedelta(seconds=delta)
    #         windows_quarter_delta = datetime.timedelta(seconds=config['windows_size']/2)
            ro_start_time = ob_start_time - delta
            ro_end_time = ob_end_time + delta

            for name, ro_df in ro_raw_namedfs.items():
                eary_time = ro_df.loc[(ro_df['timestamp'] >= ro_start_time) & (ro_df['timestamp'] <= ro_end_time) , ['eary', 'timestamp']]
                eary_time = eary_time.reset_index(drop=True)

                acc = (eary_time['eary']*camera_dire_forEarY).values
                acc = savgol_filter(acc, 31, 4)
                time = eary_time['timestamp']
                #Suppose intial velocity
                v = [0]    
                for j in range(0, len(acc)-1):
                    v.append(acc[j]*((time[j+1]-time[j]).total_seconds()) + v[j])


                x = np.linspace(0, 2*np.pi, len(v))
                y = np.sin(x)
                simV = y
                dy = np.zeros(y.shape, np.float)
                dy[0:-1] = np.diff(y)/np.diff(x)
                dy[-1] = (y[-1]-y[-2])/(x[-1]-x[-2])
                simAcc = dy
                
                #plot image
#                 plt.plot(acc, label='Acc')
#                 plt.plot(v, label='Velocity')
#                 plt.plot(simV, label='Simulated Velocity')
# #                 plt.plot(simAcc, label='Simulated Acc')
#                 plt.legend(loc='best')
#                 plt.title(name)
#                 plt.show() 
            
                #plot image double y-axes
                fig = plt.figure(figsize=(6,5))
                plt.yticks(fontsize=14)
                plt.xticks(fontsize=14)
                ax = fig.add_subplot(111)
                ax.plot(acc, '-C0', label = 'Acc')
                ax2 = ax.twinx()
                ax2.plot(v, '-C1', label = 'Velocity')
                ax2.plot(simV, '-C2', label = 'Simulated Velocity')
                ax.legend(loc='upper left',prop={'size': 14})
                ax2.legend(loc='upper right',prop={'size': 14})
#                 ax.grid()
                ax.set_xlabel("Time slot", fontsize=16)
                ax.set_ylabel(r"Acceleration ($m/s^2$)", fontsize=16)
                ax2.set_ylabel(r"Velocity ($m/s$)", fontsize=16)
#                 ax.yticks(fontsize=14)
                plt.yticks(fontsize=14)
                plt.xticks(fontsize=14)
#                 plt.title(name)
                plt.show()

                acc_rs = acc.reshape(-1, 1)
                v_rs = np.array(v).reshape(-1, 1)
                simAcc_rs = np.array(simAcc).reshape(-1, 1)
                simV_rs = np.array(simV).reshape(-1, 1)
#                 dist_norm2 = np.linalg.norm(acc_rs - simAcc_rs, ord=None)
#                 dist_dtw, cost, acc, path = dtw(acc_rs, simAcc_rs, dist=lambda acc_rs, simAcc_rs: np.linalg.norm(acc_rs - simAcc_rs, ord=None))
#                 dist_norm2 = np.linalg.norm(v_rs - simV_rs, ord=None)
#                 dist_dtw, cost, acc, path = dtw(v_rs, simV_rs, dist=lambda v_rs, simV_rs: np.linalg.norm(v_rs - simV_rs, ord=None))
                score = np.correlate(v,simV)
                print('Correlation:', score)
#                 score = 1/dist_norm2
#                 print('Norm2 Distance:', dist_norm2)
#                 print('DTW Distance:', dist_dtw)
#                 print('Score:', score)
                VC_result.append([name, what, score[0]].copy())
        VC_results[times] = VC_result
    return VC_results