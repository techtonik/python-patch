cd ..
python -m coverage run tests/run_tests.py
python -m coverage html -d tests/coverage
python -m coverage report -m
