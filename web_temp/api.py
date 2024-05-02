from flask import Flask, render_template, request, jsonify, Blueprint
import psycopg2
#from library import create_app2
from library.data import services

db_name = "demo"
db_user = "postgres"
db_password = "zenon"
db_host = "localhost"
db_port = "5432"
tb_name = "infor"

def connectDB():
    conn = psycopg2.connect(
        dbname=db_name,
        user=db_user,
        password=db_password,
        host=db_host,
        port=db_port
    )
    return conn

def create_json_file(data_input):
    result = {"link": "/get-list-node", "items": []}
    for subdata in data_input:
        
        item = {}
        idx = 0
        
        item["id"] = str(subdata[idx])
        idx += 1
        item["p"] = str(subdata[idx])
        idx += 1
        item["u"] = str(subdata[idx])
        idx += 1
        item["i"] = str(subdata[idx])
        idx += 1
        item["status"] = str(subdata[idx])
        idx += 1
        item["name"] = str(subdata[idx])
        idx += 1
        item["quantity"] = str(subdata[idx])
        result["items"].append(item)
    return result
 
app = Flask(__name__)
    
@app.route('/')
def index():
    return render_template('index.html')
    
@app.route("/get-list-node")
def get_list_node_route():
    result = services.get_list_node()
    result = create_json_file(result)
    return result   