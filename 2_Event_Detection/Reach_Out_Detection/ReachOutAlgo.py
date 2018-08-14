
# coding: utf-8

# In[12]:

import pandas as pd

df = pd.read_csv('C:\Research_temp\data\KinectData\OralDefense\VSFile.csv', header=None)

df.columns=['WristRightX', 'WristRightY', 'WristRightZ', 
            'WristLeftX', 'WristLeftY', 'WristLeftZ', 
            'ElbowRightX', 'ElbowRightY', 'ElbowRightZ',
           'ElbowLeftX', 'ElbowLeftY', 'ElbowLeftZ',
           'ShoulderRightX', 'ShoulderRightY', 'ShoulderRightZ',
           'ShoulderLeftX', 'ShoulderLeftY', 'ShoulderLeftZ',
            'SkeletonId', 'HeadX_Video', 'HeadY_Video', 'TimeStamp']
df.head()


# # Calculate percentage of reach out

# In[15]:

percentage = 100 - abs(df['ShoulderRightY'] - df['WristRightY'])*100


# # Save to csv include percentage, headX_video, headY_video

# In[25]:

result = pd.concat([percentage, df[['HeadX_Video', 'HeadY_Video', 'TimeStamp']]], axis=1)

result.to_csv('C:\Research_temp\data\Result\ReachOutResult.csv', header=None, index=False)


# In[17]:




# In[21]:

percentage


# In[ ]:



