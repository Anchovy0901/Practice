import cv2
import numpy as np

img = cv2.imread("D:/Show/test.bmp")


cropImg = img[ 1269:1269+1024,1262:1262+1024]
cv2.namedWindow('image',0)#O表示显示窗口可以随意手动调节，1
cv2.imwrite("D:/Show/1.bmp", cropImg)
cv2.imshow('image',cropImg)
cv2.waitKey()
cv2.destroyAllWindows()

