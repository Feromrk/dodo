import csv
import json

import flask
from flask import request, Response
import cerberus


app = flask.Flask(__name__)
app.config['DEBUG'] = True

db_filename = 'db.csv'

@app.route('/update-sensor', methods=['GET'])
def get_sensor_update():
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
        'hum_in': {
            'type': 'float',
            'anyof': [
                {'min': 0, 'max': 100},
                {'allowed': [-1]}
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
        }
    }

    v = cerberus.Validator(schema, allow_unknown=True)
    if not v.validate(request.args):
        return Response(json.dumps(v.errors), status=400, content_type='application/json')

    args = v.document

    with open(db_filename, 'a') as f:
        fieldnames = [
            'timestamp', 
            'temp_in',
            'temp_out',
            'hum_in',
            'rpi_state',
            'bat'
        ]
        writer = csv.DictWriter(f, fieldnames=fieldnames, extrasaction='ignore')

        if f.tell() == 0:
            writer.writeheader()

        writer.writerow(args)

    return Response('{"tasks":[]}', status=200, content_type='application/json')


@app.route('/sensor-data', methods=['GET'])
def get_sensor_data():

    with open(db_filename) as f:
        fieldnames = [
            'timestamp', 
            'temp_in',
            'temp_out',
            'hum_in',
            'rpi_state',
            'bat'
        ]
        reader = csv.DictReader(f, fieldnames=fieldnames)
        next(reader) #skip first row with fieldnames

        r_dict = {}
        for row in reader:
            r_dict[row['timestamp']] = [
                row['temp_in'],
                row['temp_out'],
                row['hum_in'],
                row['rpi_state'],
                row['bat']
            ]
        return Response(json.dumps(r_dict), status=200, content_type='application/json')
#app.run(host='192.168.2.117', port=5000)
app.run(host='0.0.0.0', port=5000)