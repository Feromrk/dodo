import csv
import json
import os

import flask
from flask import request, Response, send_file

import cerberus

import glob
import natsort


app = flask.Flask(__name__)
app.config['DEBUG'] = True

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

    return Response('{"tasks":[]}', status=200, content_type='application/json')


@app.route('/sensor-data', methods=['GET'])
def get_sensor_data():

    try:
        with open(db_filename) as f:
            reader = csv.DictReader(f, fieldnames=db_fieldnames)
            next(reader) #skip first row with fieldnames

            r_dict = {}
            for row in reader:
                if int(row['fw_version']) > 0:
                    r_dict[row['timestamp']] = [
                        row['temp_in'],
                        row['temp_out'],
                        row['rpi_state'],
                        row['bat'],
                        row['fw_version'],
                    ]
            
            resp = Response(json.dumps(r_dict), status=200, content_type='application/json')
            resp.headers['Access-Control-Allow-Origin'] = '*'
            return resp

    except FileNotFoundError:
        return Response("No database", status=500)

@app.route('/sensor-firmware', methods=['GET'])
def get_sensor_firmware():

    print(request.headers)

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
    

app.run(host='192.168.2.117', port=5000)
#app.run(host='0.0.0.0', port=5000)