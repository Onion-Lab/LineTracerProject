# from IPython.display import Image
# from IPython.display import display 
from PIL import Image
from yolo import YOLO
import numpy as np
from keras.models import load_model
import serial
import time
import cv2



yolo = YOLO(model_path='yolov3.h5', classes_path='model_data/coco_classes.txt')

def do_object_detection(image):

    pil_image=Image.fromarray(image)

    result_label, result_object_coordinate = yolo.detect_image(pil_image)
    
    return result_label


cap = cv2.VideoCapture(0)

if cap.isOpened():
    ret, a = cap.read()

    # 장치관리자에서 아두이노 장치를 확인 후, COM[번호]에 맞게 수정.
    ser = serial.Serial('COM9', 9600)
    while ret:
        ret, a = cap.read()
        cv2.imshow("camera", a)
        result = do_object_detection(a)
        print(result)
        if cv2.waitKey(1) & 0xFF == 27:
            break

        if ser.writable():
            if len(result) > 0 :
                if result[0] == 'knife':
                    print("test")
                    ser.write(b'1')
                elif result[0] == 'spoon':
                    ser.write(b'2')
                elif result[0] == 'fork' :
                    ser.write(b'3')

            

cap.release()
cv2.destroyAllWindows()