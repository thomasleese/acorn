#!/usr/bin/env python
from pathlib import Path
import subprocess


def test(filename):
    print(filename)

    executable_name = filename.name.split('.')[0]

    try:
        subprocess.run(['../build/acorn', filename],
                       check=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print(e.output.decode('UTF-8'))
        return False

    subprocess.check_call(['./' + executable_name])

    Path(executable_name).unlink()

    return True


for filename in Path('.').glob('*.acorn'):
    if not test(filename):
        break
