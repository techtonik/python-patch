from distutils.core import setup

setup(
    name='{{ module }}',
    version='{{ version }}',
    author='anatoly techtonik <techtonik@gmail.com>',
    url='https://github.com/techtonik/python-patch/',

    description='Patch utility to apply unified diffs',
    license='MIT',

    py_modules=['{{ module }}'],

    classifiers=[
        'Classifier: Programming Language :: Python :: 2 :: Only',
    ],
)
