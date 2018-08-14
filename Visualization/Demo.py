import cv2
import pandas as pd

cap = cv2.VideoCapture('C:/Users/NCTU/Desktop/WhoTakeWhat/data/Skeleton/Video_Demo.avi')

fps = cap.get(cv2.CAP_PROP_FPS)
size = (int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)),   
        int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))) 

out = cv2.VideoWriter('C:/Users/NCTU/Desktop/WhoTakeWhat/data/result/Visualization/output.avi', cv2.VideoWriter_fourcc(*'MJPG'), fps, size)

pid_result = pd.read_csv('C:/Users/NCTU/Desktop/WhoTakeWhat/data/result/PID/result.csv', header=None)
pid_result.columns = ["timestamp" if x==pid_result.shape[1]-1 else ("a%d" % x) for x in range(pid_result.shape[1])]
pid_result['timestamp'] = pd.to_datetime(pid_result["timestamp"], format='%H:%M:%S:%f')
pid_result_gen = pid_result.itertuples()

matching_result = pd.read_csv('C:/Users/NCTU/Desktop/WhoTakeWhat/data/result/Matching/match_res.csv', header=None)
matching_result.columns = ['whoTakeWhat', 'timestamp']
matching_result['timestamp'] = pd.to_datetime(matching_result['timestamp'], format='%Y-%m-%d %H:%M:%S.%f')

frame_time = pd.read_csv('C:/Users/NCTU/Desktop/WhoTakeWhat/data/Skeleton/frameTime.csv', header=None)
frame_time.columns = ['frame', 'timestamp']
frame_time_gen = frame_time.itertuples()

while(True):
    # 從攝影機擷取一張影像
    ret, frame = cap.read()
    # 顯示圖片
    
    if ret:
        pid_line = next(pid_result_gen)[1:]
        IDs = [pid_line[x] for x in range(0, len(pid_line)-1, 6)]
        headXs = [pid_line[x] for x in range(1, len(pid_line)-1, 6)]
        headYs = [pid_line[x] for x in range(2, len(pid_line)-1, 6)]
        timestamp = pid_line[-1]
        whoTakeWhats = ''
        for index, mat_row in matching_result.iterrows():
            if(mat_row['timestamp']<timestamp):
                whoTakeWhats = ('%s\n%s' % (whoTakeWhats, mat_row['whoTakeWhat']))
        for ID, x, y in zip(IDs, headXs, headYs):
            x = int(x)
            y = int(y)
            cv2.putText(frame, ID, (x, y - 20), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 100, 0), 2)
            
        for i, txt in enumerate(whoTakeWhats.split('\n')):
            wtwy0, wtwdy = 0, 25
            wtwy = wtwy0+i*wtwdy
            cv2.putText(frame, txt, (0, wtwy), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (10, 152, 10), 2)
        
        out.write(frame)
        cv2.imshow('Demo', frame)
        
    else:
        break

    # 若按下 q 鍵則離開迴圈
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break


# 釋放攝影機
cap.release()
out.release()
# 關閉所有 OpenCV 視窗
cv2.destroyAllWindows()

