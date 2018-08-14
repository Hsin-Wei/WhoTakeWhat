#!flask/bin/python
from flask import Flask
from flask import request
from datetime import datetime
import pandas as pd

app = Flask(__name__)

# @app.route('/phone/<string:data>', methods = ['POST', 'GET'])
# def index():
#     print("ind")
#     return "Hi"

@app.route('/time', methods = ['GET'])
def time():
    t = datetime.now().strftime('%H:%M:%S:%f')
    t = t[:-3]
    return t

@app.route('/phone/<string:name>', methods = ['POST'])
def csv(name):
    data = request.stream.read()
#     print(data)
    file = open("C:/Users/NCTU/Desktop/WhoTakeWhat/data/IMU/forRO/testing/%s.csv" % name, "w")
    file.write(data.decode("utf-8"))
    
    df = pd.read_csv('C:/Users/NCTU/Desktop/WhoTakeWhat/data/IMU/forRO/testing/%s.csv' % name, header=None)
    df.columns = ['earx', 'eary', 'earz', 'accx', 'accy', 'accz', 'gyrox', 'gyroy', 'gyroz', 'timestamp']
    df.to_csv('C:/Users/NCTU/Desktop/WhoTakeWhat/data/IMU/forPID/%s.csv' % name, columns=['earx', 'eary', 'earz', 'timestamp'], header=None, index=None)
    
    return ""

if __name__ == '__main__':
    app.run(host='192.168.1.126', port=8889, debug=True) #Inner wifi
#     app.run(host='192.168.50.124', port=8889, debug=True) #807 wifi