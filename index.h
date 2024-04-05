const char *HTML_CONTENT = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Centimeter Grid</title>
<style>
    canvas {
        border: 1px solid #000;
        
    }
    /* Style for vertical slider */
    .vertical-slider {
        writing-mode: bt-lr; /* Vertical orientation */
        -webkit-appearance: slider-vertical; /* For WebKit browsers */
        width: 20px; /* Adjust width as needed */
        height: 800px; /* Adjust height as needed */
    }

</style>
</head>
<body>
<canvas id="canvas" width="1200" height="1000"></canvas>

<!-- Vertical slider for z value -->
<input id="zSlider" class="vertical-slider" type="range" min="0" max="16" value="8">

<script>
    // Initialize canvas and context
    var canvas = document.getElementById('canvas');
    var ctx = canvas.getContext('2d');
    var canvasWidth = canvas.width;
    var canvasHeight = canvas.height;
    var originX = 50; // X-coordinate of the origin in pixels
    var originY = 50; // Y-coordinate of the origin in pixels
    var cmWidth = 22; // Width of canvas in centimeters
    var cmHeight = 18; // Height of canvas in centimeters
    var pixelsPerCm = 50; // Pixels per centimeter
    var zValue = 2; // Initial z value

    // Function to draw centimeter grid
    function drawGrid() {
        ctx.strokeStyle = '#ccc'; // Grid color
        ctx.lineWidth = 1;

        // Draw horizontal grid lines and write Y values
        for (var y = 0; y <= cmHeight * pixelsPerCm; y += pixelsPerCm) {
            ctx.beginPath();
            ctx.moveTo(originX, originY + y);
            ctx.lineTo(originX + cmWidth * pixelsPerCm, originY + y);
            ctx.stroke();
            // Write Y value
            ctx.font = '12px Arial';
            ctx.fillStyle = '#000';
            ctx.textAlign = 'end';
            ctx.textBaseline = 'middle';
            ctx.fillText(( y / pixelsPerCm).toFixed(1) + ' ', originX - 5, originY + y);
        }

        // Draw vertical grid lines and write X values
        for (var x = 0; x <= cmWidth * pixelsPerCm; x += pixelsPerCm) {
            ctx.beginPath();
            ctx.moveTo(originX + x, originY);
            ctx.lineTo(originX + x, originY + cmHeight * pixelsPerCm);
            ctx.stroke();
            // Write X value
            ctx.font = '12px Arial';
            ctx.fillStyle = '#000';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'top';
            ctx.fillText((x / pixelsPerCm).toFixed(1) + ' ', originX + x, originY + cmHeight * pixelsPerCm + 5);
        }
    }

    // Function to convert pixels to centimeters
    function pixelsToCm(pixels) {
        return pixels / pixelsPerCm;
    }

    // Function to draw shapes at specified coordinates
    function drawShapes(x, y,z) {
        var size = 20; // Size of the shape (in pixels)
        ctx.fillStyle = '#00f'; // Blue color
        ctx.fillRect(originX + x - size / 2, originY + y - size / 2, size, size);
        sendCoordinates(pixelsToCm(originX + x), pixelsToCm(originY + y),z ); // Send coordinates to WebSocket server
    }

    // Function to send coordinates to WebSocket server
    function sendCoordinates(x, y,z) {
       var ws = new WebSocket("ws://" + window.location.hostname + ":81");

       ws.onopen = function() {
        document.getElementById("ws_state").innerText = "CONNECTED";
        var message = JSON.stringify({ "x": Math.floor(x), "y": Math.floor(y), "Z": Math.floor(z) });
        ws.send(message);
       };

       ws.onclose = function() {
        document.getElementById("ws_state").innerText = "CLOSED";
       };

       ws.onerror = function() {
        alert("WebSocket error occurred.");
       };
}

    // Event listener for z slider change
    document.getElementById('zSlider').addEventListener('input', function(event) {
        z = parseInt(event.target.value);
        // Update zValue display or perform other actions as needed
        //z = pixelsToCm(z).toFixed(2);

    });

    // Event listener for mouse movement
    canvas.addEventListener('mousemove', function(event) {
        var rect = canvas.getBoundingClientRect();
        var mouseX = Math.round((event.clientX - rect.left));
        var mouseY = Math.round((event.clientY - rect.top));
        var cmX = pixelsToCm(mouseX - originX).toFixed(2);
        var cmY = pixelsToCm(mouseY - originY).toFixed(2);
        document.getElementById('coordinates').innerText = 'X: ' + cmX + ' cm, Y: ' + cmY + ' cm Z: '+z;
    });

    // Event listener for mouse click (draw shapes when mouse button is pressed)
    canvas.addEventListener('mousedown', function(event) {
        var rect = canvas.getBoundingClientRect();
        var mouseX = Math.round((event.clientX - rect.left));
        var mouseY = Math.round((event.clientY - rect.top));
        var cmX = pixelsToCm(mouseX - originX).toFixed(2);
        var cmY = pixelsToCm(mouseY - originY).toFixed(2);
        drawShapes(mouseX - originX, mouseY - originY,z);
        document.getElementById('coordinates').innerText = 'X: ' + cmX + ' cm, Y: ' + cmY + ' cm Z: '+z;

    
    });

    // Draw the initial grid
    drawGrid();
</script>

<div id="coordinates"></div>
<div id="ws_state"></div>
</body>
</html>
)=====";