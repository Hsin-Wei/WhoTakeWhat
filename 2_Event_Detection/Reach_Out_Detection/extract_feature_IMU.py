import numpy as np
import pickle
from scipy.stats import skew
from scipy.stats import kurtosis


config_path = './config/'
config = pickle.load(open(config_path + 'config.pkl', 'rb'))
sampleRate_IMU_acc = config['sampling_rate']
windowSize_sec = config['windows_size']

def extract_feature_IMU(IMU_sample): #input np.array shape = (300, 6)

    ##Mean (1, 6)
    IMU_mean = np.mean(IMU_sample, axis=0) 
    
    ##Skewness
    IMU_skew = skew(IMU_sample, axis=0)
    
    ##Kurtosis
    IMU_kurtosis = kurtosis(IMU_sample, axis=0)
    
    ##Mean Magnitude (1,)
    summ = 0.0
    for IMU_sample_sample in IMU_sample:
        tempSqur = 0.0
        for i in range(0, len(IMU_sample_sample)):
            tempSqur += IMU_sample_sample[i]**2
        summ = tempSqur**0.5
    IMU_MM = (summ/IMU_sample.shape[0])

    ##Variance (1, 6)
    IMU_var = np.var(IMU_sample, axis=0) 

    ##Median (1, 6)
    IMU_median = np.median(IMU_sample, axis=0)
    
    ##Energy (1, 6)
    IMU_energy = []
    for X in IMU_sample.T:
        sumTem = 0.0
        for x in X:
            sumTem = sumTem + (x*x)
        IMU_energy.append(sumTem/IMU_sample.shape[0])
    
    #-------------------------------------------
    #Covariance
    IMU_sample_T = IMU_sample.T
    cov = np.cov(IMU_sample_T)
    il = np.tril_indices(cov.shape[0], k=-1)
    IMU_cov = cov[il]
    
#     #Correlation
#     IMU_sample_T = IMU_sample.T
#     coef = np.corrcoef(IMU_sample_T)
#     il = np.tril_indices(coef.shape[0], k=-1)
#     IMU_coef = coef[il]
    
    #Zero Crossing Rate
    IMU_zero_cross = []
    IMU_sample_T = IMU_sample.T
    for sample in IMU_sample_T:
        IMU_zero_cross.append(((sample[:-1] * sample[1:]) < 0).sum())
        
    #Mean Crossing Rate
    IMU_mean_cross = []
    IMU_sample_T = IMU_sample.T
    for index, sample in enumerate(IMU_sample_T):
        sample += IMU_mean[index]
        IMU_mean_cross.append(((sample[:-1] * sample[1:]) < 0).sum())
    #-------------------------------------------
    #Time feature

    ##Mean of 1 sec (1, 3*windowSize)...
    #feature count is windowsSize/sec. 
    #Each of mean of reaching out are 100 samples.
    oneSample = []
    for start in range(0, windowSize_sec*sampleRate_IMU_acc, sampleRate_IMU_acc):
        end = start + sampleRate_IMU_acc
        tempSamples = IMU_sample[start:end]
        oneSample.extend(np.mean(tempSamples, axis=0).tolist())
        oneSample.extend(skew(tempSamples, axis=0).tolist())
        oneSample.extend(kurtosis(tempSamples, axis=0).tolist())
        oneSample.extend(np.var(tempSamples, axis=0).tolist())
        oneSample.extend(np.median(tempSamples, axis=0).tolist())
        #
        ##Mean Magnitude
        summ = 0.0
        for IMU_sample_sample in tempSamples:
            tempSqur = 0.0
            for i in range(0, len(IMU_sample_sample)):
                tempSqur += IMU_sample_sample[i]**2
            summ = tempSqur**0.5
        oneSample.append(summ/tempSamples.shape[0])
        ##Energy (1, 6)
        for X in tempSamples.T:
            sumTem = 0.0
            for x in X:
                sumTem = sumTem + (x*x)
            oneSample.append(sumTem/tempSamples.shape[0])
        ##Covariance
        one_sample_T = tempSamples.T
        one_cov = np.cov(one_sample_T)
        one_il = np.tril_indices(one_cov.shape[0], k=-1)
        oneSample.extend(one_cov[one_il])

#         ##Correlation
#         one_sample_T = tempSamples.T
#         one_coef = np.corrcoef(one_sample_T)
#         one_il = np.tril_indices(one_coef.shape[0], k=-1)
#         oneSample.extend(one_coef[one_il])
        
        #Zero Crossing Rate
        one_sample_T = tempSamples.T
        for sample in one_sample_T:
            oneSample.append(((sample[:-1] * sample[1:]) < 0).sum())

        #Mean Crossing Rate
        one_sample_T = tempSamples.T
        for index, sample in enumerate(one_sample_T):
            sample += np.mean(tempSamples, axis=0)[index]
            oneSample.append(((sample[:-1] * sample[1:]) < 0).sum())  

        IMU_Of1sec = oneSample

    #combine together
    features = []
    features.extend(IMU_mean.tolist())
    features.extend(IMU_skew.tolist())
    features.extend(IMU_kurtosis.tolist())
    features.append(IMU_MM)
    features.extend(IMU_var.tolist())
    features.extend(IMU_median.tolist())
    features.extend(IMU_energy)
    features.extend(IMU_cov.tolist())
#     features.extend(IMU_coef.tolist())
    features.extend(IMU_zero_cross)
    features.extend(IMU_mean_cross)
    features.extend(IMU_Of1sec)
    
    return features