firebase.initializeApp(firebaseConfig);
const database = firebase.database();
let auth = firebase.auth();

let userId = null;
let chart;
let temperatureData = [];
let pressureData = [];
let brightnessData = [];

auth.onAuthStateChanged(handleAuthStateChange);

document.addEventListener('DOMContentLoaded', function() {
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
                    console.error('Login error:', error);
                });
        });
    } else {
        console.error('Login form not found');
    }
});

const logoutButton = document.getElementById('logoutButton');
if (logoutButton) {
    logoutButton.addEventListener('click', function () {
        auth.signOut().then(() => {
            console.log('User signed out successfully');
        }).catch((error) => {
            console.error('Error signing out:', error);
        });
    });
} else {
    console.error('Logout button not found');
}

function handleAuthStateChange(user) {
    if (user) {
        currentUser = user;
        userId = user.uid;

        const signupContainer = document.querySelector('.signup-container');
        const loginContainer = document.querySelector('.login-container');
        const content = document.getElementById('content');

        if (signupContainer) signupContainer.classList.add('hidden');
        if (loginContainer) loginContainer.classList.add('hidden');
        if (content) content.classList.remove('hidden');

        fetchTemperature();
        fetchPressure();
        fetchBrightness();
        initializeDevice1();
        initializeDevice2();
        initializeIntensitySlider();
        readInstantDate();
    } else {
        currentUser = null;
        userId = null;

        const content = document.getElementById('content');
        const signupContainer = document.querySelector('.signup-container');
        const loginContainer = document.querySelector('.login-container');

        if (content) content.classList.add('hidden');
        if (signupContainer) signupContainer.classList.add('hidden');
        if (loginContainer) loginContainer.classList.remove('hidden');
    }
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
        series: [{
            name: 'Température',
            data: data,
            color: '#e74c3c'
        }]
    });
}

function updateTemperatureChart() {
    const numPoints = document.getElementById('temperature-points').value;
    createTemperatureChart(parseInt(numPoints));
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
        series: [{
            name: 'Pression',
            data: data,
            color: '#3498db'
        }]
    });
}

function updatePressureChart() {
    const numPoints = document.getElementById('pressure-points').value;
    createPressureChart(parseInt(numPoints));
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
        series: [{
            name: 'Luminosité',
            data: data,
            color: '#f1c40f'
        }]
    });
}

function updateBrightnessChart() {
    const numPoints = document.getElementById('brightness-points').value;
    createBrightnessChart(parseInt(numPoints));
}

function initializeDevice1() {
    const switch1 = document.querySelector('#device1Switch');
    const deviceRef = firebase.database().ref('utilisateurs/' + userId + '/instant_data/finder1');

    deviceRef.once('value', (snapshot) => {
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

    deviceRef.once('value', (snapshot) => {
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

    intensityRef.once('value', (snapshot) => {
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

    if (temperature > 50 || gas > 1500) {
        dangerMessage.textContent = '⚠️ ALERTE: Conditions dangereuses détectées! Température élevée ou niveau de gaz critique. Évacuez immédiatement et contactez les services d\'urgence.';
        dangerMessage.style.display = 'block';
    } else {
        dangerMessage.style.display = 'none';
    }
}