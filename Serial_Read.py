from asyncio.windows_events import NULL
import serial
import datetime 
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import f1_score
from sklearn.compose import ColumnTransformer
from sklearn.pipeline import Pipeline
from sklearn.impute import SimpleImputer
import math 

DATASET_PATH = "C:/Users/vasil/Documents/data.csv"

data = pd.read_csv(DATASET_PATH,index_col = False)
X = data.drop("result",axis=1)
y = data["result"]
X_train, X_test, y_train, y_test = train_test_split(X,y, test_size = 0.33,)

numeric_features = ["Temp","Speed","InTemp","Humidity","ApparenTemp"]
numeric_transformer = Pipeline([('imputer',SimpleImputer())])

preprocessor = ColumnTransformer(
    transformers=[
        ("num", numeric_transformer, numeric_features)
    ]) 

classifier = DecisionTreeClassifier(criterion="gini")

clf = Pipeline(
    steps=[("preprocessor",preprocessor),("classifier", classifier)]
)

clf.fit(X_train, y_train)
y_prediction = clf.predict(X_test)
score = f1_score(y_test,y_prediction)

t = datetime.datetime.now()
tmp = t
diff = tmp - t

ser = serial.Serial('COM7', 9600, timeout=1)
line = ser.readline()

def learning(data):
    p = data[4]/100*6.105*math.exp(17.27*data[1]/(237.7+data[1]))
    apparent_temp = data[1] + 0.33*p - 0.7*data[2] - 4
    X = np.concatenate(data,apparent_temp)
    return clf.predict(X)

c = 0
result = 1
data = []
while True:
    while diff.total_seconds() < 20 & c == 0:
        line = ser.readline()
        decoding = line.decode()
        striping = decoding.strip()
        striping = striping.split(" ")
        print(striping)
        data.append(striping)
        tmp = datetime.datetime.now()    
        diff = tmp - t    
    c = 1
    data = np.median(data,axis=1)
    result = learning(data)
    
    if result == 1:
        ser.write('1'.encode())
    else:
        ser.write('0'.encode())
    
    if not ser.readline():
        c = 0 
    data = []
    t = datetime.datetime.now()
    tmp = t