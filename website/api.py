from flask import Flask, render_template
from library import services
import json


def createJsonNodesFile(data_input):
    result = {"link": "/get-list-node", "items": []}
    for subdata in data_input:

        result["items"].append(json.loads(subdata))

    return result


###########################

app = Flask(__name__)


@app.route('/')
def index():
    return render_template('index.html')


@app.route("/get-list-node")
def getNodeList_route():
    result = services.getNodeList()
    result = createJsonNodesFile(result)
    return result


@app.route("/get-list-node/<string:node_id>/<string:action>", methods=['PATCH'])
def updateNodeStatus_route(node_id, action):
    if action == "close":
        services.changeNodeStatus(True, node_id)
    elif action == "open":
        services.changeNodeStatus(False, node_id)

    result = services.getNodeList()
    result = createJsonNodesFile(result)
    return result
