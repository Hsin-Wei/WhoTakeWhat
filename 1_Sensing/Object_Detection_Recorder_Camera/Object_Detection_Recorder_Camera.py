import cv2
import pandas as pd
from datetime import datetime
import time

# Parameters
path = 'C:/Users/NCTU/Desktop/WhoTakeWhat/data/Camera/' #Store frame Path
countdown = 5

# 選擇第一隻攝影機
cap = cv2.VideoCapture(0)
# cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
# cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

# 取得影像的尺寸大小
width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
print("Image Size: %d x %d" % (width, height))
print('Press \'s\' to store')
print('Press \'q\' to exit without storing')

frames = []
csv_result = []
i = 0

while (countdown > 0):
    print(countdown)
    countdown = countdown - 1
    time.sleep(1)

while(True):
    # 從攝影機擷取一張影像
    ret, frame = cap.read()
    
    if ret:
        frames.append(frame)
        t = datetime.now().strftime('%H:%M:%S:%f')
        t = t[:-3]
        csv_result.append([i, t])
        i = i+1
        # 顯示圖片
        cv2.namedWindow('Object Demo', cv2.WINDOW_NORMAL)
        cv2.resizeWindow('Object Demo', 640, 480)
        cv2.imshow('Object Demo', frame)
        

    if cv2.waitKey(1) & 0xFF == ord('s'):
        # 釋放攝影機
        cap.release()
        # 關閉所有 OpenCV 視窗
        cv2.destroyAllWindows()
        print('Storing frames')
        for num, frame in enumerate(frames):
            cv2.imwrite(path + '%s.jpg' % (num) ,frame)

        df = pd.DataFrame(csv_result)
        df.to_csv(path + 'original.csv', index=None, header=None)
        print('Finish')
        break

    # 若按下 q 鍵則離開迴圈
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break