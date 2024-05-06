import makePutRequest from "./request.js";


var column1 = document.querySelector(".column-1");
var column2 = document.querySelector(".column-2");
var Pvalue = document.querySelector(".power-value");
var Uvalue = document.querySelector(".u-value");
var Ivalue = document.querySelector(".i-value");
var Numbervalue = document.querySelector(".number-value");
var SwitchBtn = document.querySelector(".switch-btn");
var activateArr = [];
var deactivateArr = [];
var indexNow;

fetch("http://127.0.0.1:5000/get-list-node")
  .then((response) => response.json())
  .then((data) => {
    App(data.items)
  });

function refreshContent() {
  // Make a Fetch request to fetch new data from the server
  fetch('http://127.0.0.1:5000/get-list-node')
    .then(response => {
      if (!response.ok) {
        throw new Error('Network response was not ok');
      }
      return response.json();
    })
    .then(data => {
      activateArr = []
      deactivateArr = []
      // Update the DOM with the fetched data
      App(data.items)
    })
    .catch(error => {
      console.error('There was a problem with the fetch operation:', error);
    });
}

//Call refreshContent initially
refreshContent();
// setInterval(refreshContent, 1000);

function FilterData(data) {

  data.forEach((object) => {
    if (object.status == true) {
      activateArr.push(object);
    } else {
      deactivateArr.push(object);
    }
  });
  RenderColumn();
}

function RenderColumn() {
  var renderColumn1 = activateArr.map((object) => {
    return `<p class = "activate">${object.node_id}</p>`;
  });
  column1.innerHTML = renderColumn1.join("");

  var renderColumn2 = deactivateArr.map((object) => {
    return `<p class = "deactivate">${object.node_id}</p>`;
  });
  column2.innerHTML = renderColumn2.join("");
}

function HandleBtn() {
  SwitchBtn.onclick = () => {
    if (indexNow == null) {
      SwitchBtn.checked = true;
      alert("Please choose 1 node");
    } else {
      if (activateArr.length >= deactivateArr.length) {
        if (activateArr[indexNow].status && !SwitchBtn.checked) {
          activateArr[indexNow].status = false;
          var newData = { status: false };
          makePutRequest(
            `http://127.0.0.1:5000/get-list-node/${activateArr[indexNow].node_id}/open`,
            newData
          );
          // deactivateArr.push(activateArr[indexNow]);
          // activateArr.splice(indexNow, 1);
        } else {
          deactivateArr[indexNow].status = true;
          var newData = { status: true };
          makePutRequest(
            `http://127.0.0.1:5000/get-list-node/${deactivateArr[indexNow].node_id}/open`,
            newData
          );
          // activateArr.push(deactivateArr[indexNow]);
          // deactivateArr.splice(indexNow, 1);
        }
      } else {
        if (!deactivateArr[indexNow].Status && SwitchBtn.checked) {
          deactivateArr[indexNow].status = true;
          var newData = { status: true };
          makePutRequest(
            `http://127.0.0.1:5000/get-list-node/${deactivateArr[indexNow].node_id}/close`,
            newData
          );
          // activateArr.push(deactivateArr[indexNow]);
          // deactivateArr.splice(indexNow, 1);
        } else {
          activateArr[indexNow].status = false;
          var newData = { status: false };
          makePutRequest(
            `http://127.0.0.1:5000/get-list-node/${activateArr[indexNow].node_id}/close`,
            newData
          );
          // deactivateArr.push(activateArr[indexNow]);
          // activateArr.splice(indexNow, 1);
        }
      }

      indexNow = null;
      RenderColumn();
      DisplayInformation();
    }
  };
}

function DisplayInformation() {
  var ActivateNodes = document.querySelectorAll(".activate");
  ActivateNodes.forEach((node, index) => {
    node.onclick = function () {
      refreshContent();
      Pvalue.innerText = `${activateArr[index].power}W`;
      Uvalue.innerText = `${activateArr[index].voltage}V`;
      Ivalue.innerText = `${activateArr[index].current}A`;
      Numbervalue.innerText = `${activateArr[index].energy}`;
      SwitchBtn.checked = true;
      indexNow = index;
    };
  });

  var DeactivateNodes = document.querySelectorAll(".deactivate");
  DeactivateNodes.forEach((node, index) => {
    node.onclick = function () {
      refreshContent();
      Pvalue.innerText = `${deactivateArr[index].power}W`;
      Uvalue.innerText = `${deactivateArr[index].voltage}V`;
      Ivalue.innerText = `${deactivateArr[index].current}A`;
      Numbervalue.innerText = `${deactivateArr[index].energy}`;
      SwitchBtn.checked = false;
      indexNow = index;
    };
  });
}

function App(data) {
  FilterData(data);
  DisplayInformation();
  HandleBtn();
}