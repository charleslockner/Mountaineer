#!/bin/bash
find src | egrep "[.]cpp|[.]h" | xargs wc -l