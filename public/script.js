firebase.initializeApp(firebaseConfig);
const database = firebase.database();


let userId = null;
let chart;
let temperatureData = [];
let pressureData = [];
let brightnessData = [];
let selectedTiming = 5;
let timerInterval = null;


let auth = firebase.auth();
auth.onAuthStateChanged(handleAuthStateChange);

document.addEventListener('DOMContentLoaded', function () {
    const loginForm = document.getElementById('loginForm');
    if (loginForm) {
        loginForm.addEventListener('submit', function (event) {
            event.preventDefault();
            const email = document.getElementById('loginEmail').value;
            const password = document.getElementById('loginPassword').value;
            auth.signInWithEmailAndPassword(email, password)
                .then((userCredential) => {
                    const errorMessage = document.getElementById('loginErrorMessage');
                    if (errorMessage) {
                        errorMessage.textContent = '';
                    }
                })
                .catch((error) => {
                    const errorMessage = document.getElementById('loginErrorMessage');
                    if (errorMessage) {
                        errorMessage.textContent = 'Adresse mail ou mot de passe incorrect';
                    }
                });
        });
    };
    document.getElementById('contactLink').addEventListener('click', function() {
        alert("Pour contacter un conseiller, veuillez envoyer un email à: bosco.adresseprojet@gmail.com");
    });
});

function handleAuthStateChange(user) {
    if (user) {
        currentUser = user;
        userId = user.uid;

        const loginContainer = document.querySelector('.login-container');
        const content = document.getElementById('content');

        if (loginContainer) loginContainer.classList.add('hidden');
        if (content) content.classList.remove('hidden');

        setInterval(updateClock, 1000);
        updateClock();
        initializeDevice1();
        initializeDevice2();
        initializeIntensitySlider();
        readInstantDate();
        getTimeFromFirebase();


        if (navigator.geolocation) {
            navigator.geolocation.getCurrentPosition(getWeather, showError);
        } else {
            document.getElementById('city-name').textContent = "Géolocalisation non disponible";
        }


        updateDeviceCalendar('Device 1', 'device1Start', 'device1End', 'device1Auto');
        updateDeviceCalendar('Device 2', 'device2Start', 'device2End', 'device2Auto');

        document.getElementById('measureInterval').addEventListener('change', writeTimeToFirebase);

        document.querySelector('.calendar-btn1').addEventListener('click', function () {
            saveDeviceCalendar('Device 1', 'device1Start', 'device1End', 'device1Auto');
        });

        document.querySelector('.calendar-btn2').addEventListener('click', function () {
            saveDeviceCalendar('Device 2', 'device2Start', 'device2End', 'device2Auto');
        });

        const logoutButton = document.getElementById('logoutButton');
        if (logoutButton) {
            logoutButton.addEventListener('click', function () {
                auth.signOut();
            });
        }

        const resetPasswordButton = document.querySelector('.resetPasswordButton');
        const clearDataButton = document.querySelector('.clearDataButton');

        if (resetPasswordButton) {
            resetPasswordButton.onclick = resetPassword;
        }

        if (clearDataButton) {
            clearDataButton.onclick = clearData;
        }

        fetchTemperature();
        fetchPressure();
        fetchBrightness();

    } else {
        currentUser = null;
        userId = null;

        const content = document.getElementById('content');
        const loginContainer = document.querySelector('.login-container');

        if (content) content.classList.add('hidden');
        if (loginContainer) loginContainer.classList.remove('hidden');
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////////////////////////////////////////////////////////////

function resetPassword() {
    const email = auth.currentUser ? auth.currentUser.email : null;
    if (email) {
        auth.sendPasswordResetEmail(email)
            .then(() => {
                alert('Un email de réinitialisation de mot de passe a été envoyé.');
            })
            .catch((error) => {
                alert('Erreur : impossible de réinitialiser le mot de passe.');
            });
    } else {
        alert('Aucun utilisateur connecté');
    }
}

function clearData() {
    const nodeRef = firebase.database().ref('utilisateurs/' + userId + '/data');
    nodeRef.set(null)
        .then(() => {
            alert("Données supprimées avec succès");
            location.reload(true);
        })
        .catch((error) => {
            alert("Erreur lors de la suppression des données: " + error.message);
        });
}

function readInstantDate() {

    const tempRef = firebase.database().ref('utilisateurs/' + userId + '/instant_data/temperature');
    tempRef.on('value', (snapshot) => {
        const temperature = snapshot.val();
        document.getElementById('temperature').innerHTML = temperature;
        checkDanger();
    });

    const presRef = firebase.database().ref('utilisateurs/' + userId + '/instant_data/pressure');
    presRef.on('value', (snapshot) => {
        const pressure = snapshot.val();
        document.getElementById('pressure').innerHTML = pressure;
    });

    const gasRef = firebase.database().ref('utilisateurs/' + userId + '/instant_data/gas');
    gasRef.on('value', (snapshot) => {
        const gas = snapshot.val();
        document.getElementById('gas').innerHTML = gas;
        checkDanger();
    });

    const brightRef = firebase.database().ref('utilisateurs/' + userId + '/instant_data/brightness');
    brightRef.on('value', (snapshot) => {
        const brightness = snapshot.val();
        document.getElementById('brightness').innerHTML = brightness;
    });
}

function fetchTemperature() {
    const temperatureRef = firebase.database().ref('utilisateurs/' + userId + '/data/temperature');
    temperatureRef.on('value', (snapshot) => {
        const data = snapshot.val();
        temperatureData = Object.entries(data).map(([time, entry]) => ({
            x: parseInt(time) * 1000,
            y: entry.valeur
        }));
        temperatureData.sort((a, b) => a.x - b.x);
        createTemperatureChart(temperatureData.length);
    });
}

function createTemperatureChart(numPoints) {
    const data = temperatureData.slice(-numPoints);
    const container = document.getElementById('temperature-chart');

    Highcharts.setOptions({
        time: {
            timezoneOffset: new Date().getTimezoneOffset(),
            useUTC: false
        }
    });

    chart = Highcharts.chart(container, {
        chart: {
            zoomType: 'x',
            panning: true,
            panKey: 'shift',
        },
        title: {
            text: 'Évolution de la Température'
        },
        xAxis: {
            type: 'datetime',
            title: {
                text: 'Temps'
            }
        },
        yAxis: {
            title: {
                text: 'Température (°C)'
            }
        },
        tooltip: {
            shared: true,
            crosshairs: true,
            formatter: function () {
                return `<b>Temps:</b> ${Highcharts.dateFormat('%e %b %Y %H:%M', this.x)}<br/>
                        <b>Température:</b> ${this.y.toFixed(2)} °C`;
            }
        },
        series: [{
            name: 'Température',
            data: data,
            color: '#e74c3c',
            marker: {
                enabled: true,
                radius: 3
            }
        }],
        legend: {
            enabled: false
        }
    });
}

function fetchPressure() {
    const pressureRef = firebase.database().ref('utilisateurs/' + userId + '/data/pressure');
    pressureRef.on('value', (snapshot) => {
        const data = snapshot.val();
        pressureData = Object.entries(data).map(([time, entry]) => ({
            x: parseInt(time) * 1000,
            y: entry.valeur
        }));
        pressureData.sort((a, b) => a.x - b.x);
        createPressureChart(pressureData.length);
    });
}

function createPressureChart(numPoints) {
    const data = pressureData.slice(-numPoints);
    const container = document.getElementById('pressure-chart');

    Highcharts.setOptions({
        time: {
            timezoneOffset: new Date().getTimezoneOffset(),
            useUTC: false
        }
    });

    chart = Highcharts.chart(container, {
        chart: {
            zoomType: 'x',
            panning: true,
            panKey: 'shift',
        },
        title: {
            text: 'Évolution de la Pression'
        },
        xAxis: {
            type: 'datetime',
            title: {
                text: 'Temps'
            }
        },
        yAxis: {
            title: {
                text: 'Pression (Pa)'
            }
        },
        tooltip: {
            shared: true,
            crosshairs: true,
            formatter: function () {
                return `<b>Temps:</b> ${Highcharts.dateFormat('%e %b %Y %H:%M', this.x)}<br/>
                        <b>Pression:</b> ${this.y.toFixed(2)} Pa`;
            }
        },
        series: [{
            name: 'Pression',
            data: data,
            color: '#3498db',
            marker: {
                enabled: true,
                radius: 3
            }
        }],
        legend: {
            enabled: false
        }
    });
}

function fetchBrightness() {
    const brightnessRef = firebase.database().ref('utilisateurs/' + userId + '/data/brightness');
    brightnessRef.on('value', (snapshot) => {
        const data = snapshot.val();
        brightnessData = Object.entries(data).map(([time, entry]) => ({
            x: parseInt(time) * 1000,
            y: entry.valeur
        }));
        brightnessData.sort((a, b) => a.x - b.x);
        createBrightnessChart(brightnessData.length);
    });
}

function createBrightnessChart(numPoints) {
    const data = brightnessData.slice(-numPoints);
    const container = document.getElementById('brightness-chart');

    Highcharts.setOptions({
        time: {
            timezoneOffset: new Date().getTimezoneOffset(),
            useUTC: false
        }
    });

    chart = Highcharts.chart(container, {
        chart: {
            zoomType: 'x',
            panning: true,
            panKey: 'shift',
        },
        title: {
            text: 'Évolution de la Luminosité'
        },
        xAxis: {
            type: 'datetime',
            title: {
                text: 'Temps'
            }
        },
        yAxis: {
            title: {
                text: 'Luminosité (lux)'
            }
        },
        tooltip: {
            shared: true,
            crosshairs: true,
            formatter: function () {
                return `<b>Temps:</b> ${Highcharts.dateFormat('%e %b %Y %H:%M', this.x)}<br/>
                        <b>Luminosité:</b> ${this.y.toFixed(2)} lux`;
            }
        },
        series: [{
            name: 'Luminosité',
            data: data,
            color: '#f1c40f',
            marker: {
                enabled: true,
                radius: 3
            }
        }],
        legend: {
            enabled: false
        }
    });
}

function initializeDevice1() {
    const switch1 = document.querySelector('#device1Switch');
    const deviceRef = firebase.database().ref('utilisateurs/' + userId + '/instant_data/finder1');

    deviceRef.on('value', (snapshot) => {
        switch1.checked = snapshot.val() === true;
    });

    switch1.addEventListener('change', (event) => {
        deviceRef.set(event.target.checked);
    });

    deviceRef.on('value', (snapshot) => {
        switch1.checked = snapshot.val() === true;
    });
}

function initializeDevice2() {
    const switch2 = document.querySelector('#device2Switch');
    const deviceRef = firebase.database().ref('utilisateurs/' + userId + '/instant_data/finder2');

    deviceRef.on('value', (snapshot) => {
        switch2.checked = snapshot.val() === true;
    });

    switch2.addEventListener('change', (event) => {
        deviceRef.set(event.target.checked);
    });

    deviceRef.on('value', (snapshot) => {
        switch2.checked = snapshot.val() === true;
    });
}

function initializeIntensitySlider() {
    const slider = document.querySelector('#intensitySlider');
    const sliderValue = document.getElementById('sliderValue');
    const intensityRef = firebase.database().ref('utilisateurs/' + userId + '/instant_data/pwm');

    intensityRef.on('value', (snapshot) => {
        const value = snapshot.val() || 50;
        slider.value = value;
        sliderValue.textContent = value;
    });

    slider.addEventListener('input', (event) => {
        const value = event.target.value;
        sliderValue.textContent = value;
        intensityRef.set(parseInt(value));
    });

    intensityRef.on('value', (snapshot) => {
        const value = snapshot.val() || 50;
        slider.value = value;
        sliderValue.textContent = value;
    });
}

function checkDanger() {
    const temperature = parseFloat(document.getElementById('temperature').innerHTML);
    const gas = parseFloat(document.getElementById('gas').innerHTML);
    const dangerMessage = document.getElementById('dangerMessage');

    if (temperature > 50 || gas > 5000) {
        dangerMessage.textContent = '⚠️ ALERTE: Conditions dangereuses détectées! Température élevée ou niveau de gaz critique. Évacuez immédiatement et contactez les services d\'urgence.';
        dangerMessage.style.display = 'block';
    } else {
        dangerMessage.style.display = 'none';
    }
}

function updateClock() {
    const timeEl = document.querySelector('.time');
    const dateEl = document.querySelector('.date');
    const now = new Date();

    const hours = now.getHours().toString().padStart(2, '0');
    const minutes = now.getMinutes().toString().padStart(2, '0');
    const seconds = now.getSeconds().toString().padStart(2, '0');
    timeEl.textContent = `${hours}:${minutes}:${seconds}`;

    const options = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' };
    dateEl.textContent = now.toLocaleDateString('fr-FR', options);
}

function saveDeviceCalendar(deviceName, startDateId, endDateId, autoModeId) {
    const startInput = document.getElementById(startDateId);
    const endInput = document.getElementById(endDateId);
    const autoModeCheckbox = document.getElementById(autoModeId);
    const deviceRef = database.ref('utilisateurs/' + userId + '/auto/' + deviceName);

    const startDate = startInput.value ? new Date(startInput.value).getTime() : 0;
    const endDate = endInput.value ? new Date(endInput.value).getTime() : 0;
    const autoMode = autoModeCheckbox.checked;

    deviceRef.set({
        startDate: startDate,
        endDate: endDate,
        autoMode: autoMode
    });
}

function updateDeviceCalendar(deviceName, startDateId, endDateId, autoModeId) {
    const deviceRef = database.ref('utilisateurs/' + userId + '/auto/' + deviceName);

    deviceRef.on('value', (snapshot) => {
        const deviceCalendar = snapshot.val();

        if (deviceCalendar) {
            document.getElementById(autoModeId).checked = deviceCalendar.autoMode;
        } else {
            document.getElementById(autoModeId).checked = false;
        }
    });
}

function getWeather(position) {
    const apiKey = '5105d3f951aabd59a665c2f8fb72ad94';
    const lat = position.coords.latitude;
    const lon = position.coords.longitude;
    const units = 'metric';

    const apiUrl = `https://api.openweathermap.org/data/2.5/weather?lat=${lat}&lon=${lon}&units=${units}&appid=${apiKey}`;

    fetch(apiUrl)
        .then(response => response.json())
        .then(data => {
            document.getElementById('city-name').textContent = data.name;
            document.getElementById('temp').textContent = Math.round(data.main.temp) + '°C';
            document.getElementById('description').textContent = data.weather[0].description;
        })
        .catch(error => {
            console.error('Erreur lors de la récupération des données météo:', error);
            document.getElementById('city-name').textContent = "Erreur de météo";
            document.getElementById('temp').textContent = "--°C";
            document.getElementById('description').textContent = "--";
        });
}

function showError(error) {
    switch (error.code) {
        case error.PERMISSION_DENIED:
            alert("L'utilisateur a refusé la demande de géolocalisation.");
            break;
        case error.POSITION_UNAVAILABLE:
            alert("Les informations de localisation sont indisponibles.");
            break;
        case error.TIMEOUT:
            alert("La demande de géolocalisation a expiré.");
            break;
        case error.UNKNOWN_ERROR:
            alert("Une erreur inconnue s'est produite.");
            break;
    }
}

function toggleVisibility(containerId, headerElement) {
    const container = document.getElementById(containerId);
    if (container) {
        container.classList.toggle('hidden');
    }
    const arrow = headerElement.querySelector('.toggle-arrow');
    if (arrow) {
        arrow.textContent = container.classList.contains('hidden') ? '▶' : '▼';
    }
}

function writeTimeToFirebase() {

    const selectedValue = document.getElementById('measureInterval').value;
    const milliseconds = selectedValue * 60 * 1000;
    const timeRef = database.ref('utilisateurs/' + userId + '/interval');

    timeRef.set({
        interval: milliseconds,
    });
}

function getTimeFromFirebase() {

    const timeRef = database.ref('utilisateurs/' + userId + '/interval');

    timeRef.on('value', (snapshot) => {
        const intervalData = snapshot.val();
        if (intervalData && intervalData.interval) {
            const intervalInMinutes = intervalData.interval / (60 * 1000);
            document.getElementById('measureInterval').value = intervalInMinutes;
        } else {
            document.getElementById('measureInterval').value = 1;
            writeTimeToFirebase();
        }
    });
}