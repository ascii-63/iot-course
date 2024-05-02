#from library.model_ma import list_node_schema
#from library.model import data
#from library.extension import db
from flask import request, jsonify
import json
import psycopg2

#info_node = list_node_schema

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

def get_list_node():
    conn = connectDB()
    cur = conn.cursor()
    
    cur.execute(f"SELECT * FROM {tb_name}")
    list_node = list(cur.fetchall())
    
    conn.close()
    cur.close()
    if list_node:
        return list_node
    else:
        return "Nothing"

    