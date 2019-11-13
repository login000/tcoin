#!/bin/bash

/bin/cat /home/login/tcoin/*.txt | /usr/bin/paste -sd+ - | /usr/bin/bc
