from setuptools import setup

readme = open('README.rst').read()

setup(
    name='patch',
    version='1.16.0',
    author='Anatoly Techtonik',
    autor_email='techtonik@gmail.com',
    url='https://github.com/techtonik/python-patch/',
    description='Patch utility to apply unified diffs',
    license='MIT',
    py_modules=['patch'],
    long_description=readme,
    classifiers=[
        "Environment :: Console",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 2.6",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.3",
        "Programming Language :: Python :: 3.4",
        "Programming Language :: Python :: 3.5",
    ],
    entry_points={'console_scripts': 'patch = patch:main'},
    zip_safe=True,
)
