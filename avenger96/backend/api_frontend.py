import csv
import json
import os

import flask
from flask import request, Response
from flask_cors import CORS

import cerberus
import requests

app = flask.Flask(__name__)
app.config['DEBUG'] = True
CORS(app) #fix this in production

db_filename = '/var/opt/dodo/db.csv'
db_fieldnames = [
    'timestamp', 
    'temp_in',
    'temp_out',
    'rpi_state',
    'bat',
    'fw_version'
]
task_queue_file = '/var/opt/dodo/task_queue.json'

try:
    f = open(task_queue_file)
except IOError:
    # If not exists, create the file
    with open(task_queue_file, 'w+') as f:
        f.write('[]')

# ---------------------------
# ENDPOINTS FOR THE FRONTEND over HTTPS
# ---------------------------

# get collected sensor data
@app.route('/sensor-data', methods=['GET'])
def get_sensor_data():

    try:
        with open(db_filename) as f:
            reader = csv.DictReader(f, fieldnames=db_fieldnames)
            next(reader) #skip first row with fieldnames

            r_dict = {}
            for i, row in enumerate(reversed(list(reader))):
                if int(row['fw_version']) > 0:
                    r_dict[row['timestamp']] = [
                        row['temp_in'],
                        row['temp_out'],
                        row['rpi_state'],
                        row['bat'],
                        row['fw_version'],
                    ]
                if i > 400: 
                    break
            
            resp = Response(json.dumps(r_dict), status=200, content_type='application/json')
            resp.headers['Access-Control-Allow-Origin'] = '*'
            return resp

    except FileNotFoundError:
        return Response("No database", status=500)



# add a new task that will be dispatched by the esp
@app.route('/sensor-task', methods=['POST'])
def post_sensor_task():
    # json format: {'id':1, 'params':{'timeout': 5}}
    # 'id' int
    # 'params' object with param name and value
    # calling this again with the same id overwrites the params and moves the task at the end of the queue

    if not request.is_json:
        return Response(status=415)

    schema = {
        'id': {
            'type': 'integer',
            'required': True,
            'coerce': int
        },
        'params': {
            'type': 'dict',
            'keysrules': {
                'type': 'string'
            },
            'valuesrules': {
                'type': ['boolean', 'number', 'string']
            }
        },
    }

    v = cerberus.Validator(schema, allow_unknown=True)
    if not v.validate(request.get_json()):
        return Response(json.dumps(v.errors), status=400, content_type='application/json')

    request_body = v.document
    print("Request body:", request_body)

    #task_queue.clear()

    new_task = True


    #read queue from disk to memory
    with open(task_queue_file, 'r') as f:
        task_queue = json.load(f)

    if len(task_queue):
        for i, task in enumerate(task_queue):
            #if task in queue, update params
            if task['id'] == request_body['id']:
                print('updating task')
                params = request_body.get('params')
                if params:
                    task['params'] = params
                else:
                    task.pop('params', None)
                    task_queue[i] = task
                new_task = False
                #move task to the end of the list
                task_queue.append(task_queue.pop(i))
                break
    
    if new_task:
        print('adding task')
        task_queue.append(request_body)
    
    print("New task queue:", task_queue)
    
    resp = Response(json.dumps(task_queue), status=200, content_type='application/json')

    with open(task_queue_file, 'w') as f:
        json.dump(task_queue, f)

    return resp

@app.route('/camera-stream', methods=['GET'])
def get_camera_stream():

    try:
        r = requests.get('http://192.168.2.106:8081/', stream=True, timeout=(3.05, 27))

        return Response(r.iter_content(chunk_size=10*1024),
                        content_type=r.headers['Content-Type'])
    except requests.exceptions.ConnectionError:
        return Response(status=424)



if __name__ == '__main__':
    app.run(host='192.168.2.117', port=5000, ssl_context=('/etc/letsencrypt/live/visitdodo.ddns.net/fullchain.pem', '/etc/letsencrypt/live/visitdodo.ddns.net/privkey.pem'))
    #app.run(host='0.0.0.0', port=5000)