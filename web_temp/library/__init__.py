from flask import Flask, request, Blueprint
#from .data.controller import data
#from .extension import db, ma
#from .model import data
import os


""" def create_db(app):
    if not os.path.exists("library/library.db"):
        db.create_all(app = app) """ 
        
def create_app2(config_file = "config.py"):
    app = Flask(__name__)
    app.config.from_pyfile(config_file)
    # ma.init_app(app=app)
    # db.init_app(app=app)
    # create_db(app=app)
    # app.register_blueprint(data)
    return app