from distutils.core import setup

setup(
    name='{{ module }}',
    version='{{ version }}',
    author='{{ author }}',
    url='https://github.com/techtonik/python-patch/',

    description='Patch utility to apply unified diffs',
    license='{{ license }}',

    py_modules=['{{ module }}'],

    classifiers=[
        'Classifier: Programming Language :: Python :: 2',
        'Classifier: Programming Language :: Python :: 3',
    ],
)
