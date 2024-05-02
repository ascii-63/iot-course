Before installing the necessary packages for the project, ensure you have created a dedicated environment for it to avoid impacting your other projects.

To create an environment in Conda, use the following command (you may choose a different Python version, though it's recommended to use version 3.10 or higher):
```
    conda create -n name_of_your_env python==3.10
```
Once the environment is created, make sure to activate it. Use the following command to activate the environment:
```
    conda activate name_of_your_env
```
Proceed with installing the required packages:
```
    conda install --file requirements.txt
```
After completing the setup of the environment with the necessary packages, you can run the project using the following command:
```
    python3 run.py
```
Then, access the displayed URL to navigate to the website.