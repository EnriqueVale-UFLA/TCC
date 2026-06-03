import cv2
from ultralytics import YOLO

model = YOLO("yolov8n.pt")

cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Erro: não consegui abrir a webcam.")
    exit()

while True:
    ret, frame = cap.read()

    if not ret:
        print("Erro ao capturar frame.")
        break

    results = model(frame, conf=0.5)

    annotated_frame = results[0].plot()

    cv2.imshow("YOLO Webcam - Pessoa e Objetos", annotated_frame)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()