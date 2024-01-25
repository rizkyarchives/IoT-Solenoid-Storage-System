// Script.js
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    websocket.send("states");
}
  
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
} 
// Correct Pin Value
let correctPin = "1234";

let btns =
	document.getElementsByClassName(
		"pinpad-btn"
	);
let pinInput = document.getElementById(
	"pinpad-input"
);

for (let i = 0; i < btns.length; i++) {
	let btn = btns.item(i);
	if (
		btn.id &&
		(btn.id === "submit-btn" ||
			btn.id === "delete-btn")
	)
		continue;

	// Add onclick event listener to 
	// Every button from 0 - 9
	btn.addEventListener(
		"click",
		(e) => {
			pinInput.value +=
				e.target.value;
		}
	);
}

let submitBtn = document.getElementById(
	"submit-btn"
);
let delBtn = document.getElementById(
	"delete-btn"
);
let resBtn = document.getElementById(
	"reset-btn"
);
let modal =
	document.getElementById("modal");
let result =
	document.getElementById("result");
let closeBtn =
	document.getElementById("close");

submitBtn.addEventListener(
	"click",
	() => {
		if (
			!pinInput ||
			!pinInput.value ||
			pinInput.value === ""
		) {
			alert(
				"Please enter a pin first"
			);
		} else if (
			pinInput.value ===
			correctPin
		) {
			websocket.send("correct");
			alert("Correct PIN");
			resBtn.classList.remove('hidden');
			pinInput.disabled = true;
			for (let i = 0; i < btns.length; i++) {
				btns[i].disabled = true;
			}
			document.querySelector('.text-center').textContent = 'Unlocked';
		} else {
			websocket.send("incorrect");
			alert("Incorrect PIN");
		}
		// Reset the input
		pinInput.value = "";
	}
);

function onMessage(event) {
    var receivedData = event.data;
    if (receivedData === "reset") {
        // Reset PIN input and buttons
        pinInput.value = '';
        pinInput.disabled = false;
        for (let i = 0; i < btns.length; i++) {
            btns[i].disabled = false;
        }
        resBtn.classList.add('hidden');
        document.querySelector('.text-center').textContent = 'Unlock with PIN';
}   else if(receivedData === "correct"){
		resBtn.classList.remove('hidden');
		pinInput.disabled = true;
		for (let i = 0; i < btns.length; i++) {
			btns[i].disabled = true;
		}
		document.querySelector('.text-center').textContent = 'Unlocked';
	}
};

delBtn.addEventListener("click", () => {
	if (pinInput.value)
		pinInput.value =
			pinInput.value.substr(
				0,
				pinInput.value.length -
					1
			);
});

closeBtn.addEventListener(
	"click",
	() => {
		modal.style.display = "none";
	}
);

resBtn.addEventListener(
	"click",
	() => {
		websocket.send("reset");
		pinInput.value = '';
		pinInput.disabled = false;
		for (let i = 0; i < btns.length; i++) {
			btns[i].disabled = false;
		}
		resBtn.classList.add('hidden');
		document.querySelector('.text-center').textContent = 'Unlock with PIN';
	}
);
