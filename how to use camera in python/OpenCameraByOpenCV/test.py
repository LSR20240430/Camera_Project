import cv2

# 初始化摄像头（索引0 + DirectShow后端）
cap = cv2.VideoCapture(1
                       , cv2.CAP_DSHOW)

if not cap.isOpened():
    print("无法打开摄像头！")
    exit()

while True:
    ret, frame = cap.read()  # 读取一帧
    if not ret:
        print("获取帧失败")
        break

    cv2.imshow("Camera", frame)  # 显示画面
    if cv2.waitKey(1) == ord('q'):  # 按Q退出
        break

cap.release()
cv2.destroyAllWindows()