import flask
from flask import request
import csv

app = flask.Flask(__name__)
app.config['DEBUG'] = True


@app.route('/update-sensor', methods=['GET'])
def update_sensor():
    temp_in = request.args.get('temp_in')
    temp_out = request.args.get('temp_out')
    hum_in = request.args.get('hum_in')
    rpi_state = request.args.get('rpi_state')
    timestamp = request.args.get('timestamp')
    bat = request.args.get('bat')

    new_row = [timestamp, temp_in, temp_out, hum_in, rpi_state, bat]

    with open('db.csv', 'a') as f:
        writer = csv.writer(f)
        writer.writerow(new_row)

    return '{tasks:[]}'

app.run(host='192.168.2.117', port=5000)