import sys
import os
import subprocess
import shlex
import shutil
import json

# commands
arduino_cli_path = "C:\\Users\\gruber04\\Downloads\\arduino-cli_0.21.1_Windows_64bit\\arduino-cli.exe"
compilation = "{EXE} compile -b arduino:samd:nano_33_iot .\\arduino_edge_node\\"
upload = "{EXE} upload -b arduino:samd:nano_33_iot -p {PORT} .\\arduino_edge_node\\"

# template for sensors.h
with open("arduino_edge_node/sensor_template.h", "r") as f:
    sensors_header_file_template = f.read()

print(sensors_header_file_template)

# get sensors to
sensors_to_work_with = json.load(open("arduino_edge_node/sensors.json", "r"))

for sensor_id, settings in sensors_to_work_with.items():

    # load sensor description
    if os.path.exists(settings["description_file"]):
        with open(settings["description_file"], "r") as f:
            sensor_description = f.read()
    else:
        continue

    # adjust template
    sensor_header = sensors_header_file_template.format(
        SENSOR_ID=sensor_id, SENSOR_DESCRIPTION=sensor_description,
    )

    # create sensors.h
    with open("arduino_edge_node/sensor.h", "w") as f:
        sensor_description = f.write(sensor_header)

    # compile
    cmd = shlex.split(compilation.format(EXE=arduino_cli_path), posix=False)
    #subprocess.run(cmd, capture_output=True)

    # (try to) upload
    cmd = shlex.split(upload.format(EXE=arduino_cli_path, PORT=settings["port"]), posix=False)
    #subprocess.run(cmd, capture_output=True)

    # clean up
    os.remove("arduino_edge_node/sensor.h")
