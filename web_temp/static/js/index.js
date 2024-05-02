var column1 = document.querySelector(".column-1");
var column2 = document.querySelector(".column-2");
var Pvalue = document.querySelector(".power-value");
var Uvalue = document.querySelector(".u-value");
var Ivalue = document.querySelector(".i-value");
var Numbervalue = document.querySelector(".number-value");
var activateArr = [];
var deactivateArr = [];


fetch("http://127.0.0.1:5000/get-list-node")
  .then((response) => response.json())
  .then((data) => {
    App(data.items)
  });

  function FilterData(data) {
      data.forEach((object) => {
        
      if (object.status == "True") {
        activateArr.push(object);
      } else {
        deactivateArr.push(object);
      }
    });
  }

function RenderColumn() {
  var renderColumn1 = activateArr.map((object) => {
    return `<li class = "activate">${object.name}</li>`;
  });
  column1.innerHTML = renderColumn1.join("");

  var renderColumn2 = deactivateArr.map((object) => {
    return `<li>${object.name}</li>`;
  });
  column2.innerHTML = renderColumn2.join("");
}

function DisplayValue() {
  var ActivateNodes = document.querySelectorAll(".activate");

  // Sử dụng forEach để lặp qua từng phần tử activate
  ActivateNodes.forEach((node, index) => {
    node.onclick = function () {
      Pvalue.innerText = `${activateArr[index].p}W`;
      Uvalue.innerText = `${activateArr[index].u}V`;
      Ivalue.innerText = `${activateArr[index].i}A`;
      Numbervalue.innerText = `${activateArr[index].quantity}`;
    };
  });
}
function App(data){
    FilterData(data)
    RenderColumn()
    DisplayValue()
}

