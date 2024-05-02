""" import os
from dotenv import load_dotenv

load_dotenv()

SECRET_KEY = os.environ.get("KEY")
SQUALCHEMY_DATABASE_URI = os.environ.get("DATABASE_URL")
SQUALCHEMY_TRACK_MODIFICATION = False """