import csv
import json
import os

import flask
from flask import request, Response, send_file
#from flask_cors import CORS

import cerberus

import glob
import natsort

app = flask.Flask(__name__)
app.config['DEBUG'] = True
#CORS(app) #fix this in production

db_filename = 'db.csv'
db_fieldnames = [
    'timestamp', 
    'temp_in',
    'temp_out',
    'rpi_state',
    'bat',
    'fw_version'
]
firmware_folder = '/var/opt/dodo/sensor_firmware'
task_queue_file = '/var/opt/dodo/task_queue.json'

try:
    f = open(task_queue_file)
except IOError:
    # If not exists, create the file
    with open(task_queue_file, 'w+') as f:
        f.write('[]')

# ---------------------------
# ENDPOINTS FOR THE ESP8266 over plain HTTP
# ---------------------------


# esp uses this endpoint for the OTA firmware update
@app.route('/sensor-firmware', methods=['GET'])
def get_sensor_firmware():

    #print(request.headers)

    user_agent = request.headers.get('User-Agent')
    if not user_agent == 'ESP8266-http-Update':
        print('Wrong user agent!', user_agent)
        return Response('Only for ESP8266 updater!',status=403, content_type='text/plain')

    firmware_filenames = glob.glob(f"{firmware_folder}/*.bin")

    if firmware_filenames:
        latest_firmware_filename = natsort.natsorted(firmware_filenames, reverse=True)[0]
        latest_firmware_version = os.path.basename(latest_firmware_filename).split('.')[0]

    remote_firmware_version = request.headers.get('X-Esp8266-Version')

    try:
        remote_firmware_version = int(request.headers['X-Esp8266-Version'])
    except (KeyError, ValueError) as e:
        print(e)
        return Response('X-Esp8266-Version header missing', status=403, content_type='text/plain')

    if int(latest_firmware_version) > remote_firmware_version:
        return send_file(latest_firmware_filename, as_attachment=True)
    
    return Response(status=304)

# esp uses this endpoint to push new sensor data; also returns queued tasks
@app.route('/sensor-push', methods=['GET'])
def get_sensor_push():
    request.parameter_storage_class = dict

    schema = {
        'temp_in': {
            'type': 'float',
            'anyof': [
                {'min': -40, 'max': 70},
                {'allowed': [-100]}
            ],
            'required': True,
            'coerce': float
        },
        'temp_out': {
            'type': 'float',
            'anyof': [
                {'min': -40, 'max': 70},
                {'allowed': [-100]}
            ],
            'required': True,
            'coerce': float
        },
        'rpi_state': {
            'type': 'integer',
            'coerce': int,
            'required': True,
        },
        'timestamp': {
            'type': 'integer',
            'min': 1593551335,
            'required': True,
            'coerce': int
        },
        'bat': {
            'type': 'integer',
            'coerce': int,
            'required': True,
        },
        'fw_version': {
            'type': 'integer',
            'required': True,
            'coerce': int
        }
    }

    v = cerberus.Validator(schema, allow_unknown=True)
    if not v.validate(request.args):
        return Response(json.dumps(v.errors), status=400, content_type='application/json')

    args = v.document

    with open(db_filename, 'a') as f:
        writer = csv.DictWriter(f, fieldnames=db_fieldnames, extrasaction='ignore')

        if f.tell() == 0:
            writer.writeheader()

        writer.writerow(args)

        print("csv")

    with open(task_queue_file, 'r') as f:
        task_queue = json.load(f)

    print("tasks", task_queue)

    r = Response(json.dumps(task_queue), status=200, content_type='application/json')

    #clear file
    with open(task_queue_file, 'w') as f:
        f.write('[]')
    
    return r

if __name__ == '__main__':
    app.run(host='192.168.2.117', port=5001)
    #app.run(host='0.0.0.0', port=5000)