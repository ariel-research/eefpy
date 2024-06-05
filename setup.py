from setuptools import setup, find_packages  # Import setuptools

setup(
    name='eefpy',
    version='0.1.0',
    description='python eef practical solver', 
    long_description=open('README.md').read(),
    long_description_content_type='text/markdown',
    url='https://github.com/ariel-research/eefpy',
    license='GNU',
    packages=find_packages(),
    install_requires=[
        'cppyy',
    ],
)
