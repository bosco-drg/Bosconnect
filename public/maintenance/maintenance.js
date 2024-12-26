const firebaseConfig = {
    apiKey: "AIzaSyBluSTtDBKsxPEBhWRs-42WyvwxFodY8RQ",
    authDomain: "bosco-nnect.firebaseapp.com",
    databaseURL: "https://bosco-nnect-default-rtdb.europe-west1.firebasedatabase.app",
    projectId: "bosco-nnect",
    storageBucket: "bosco-nnect.appspot.com",
    messagingSenderId: "761637578037",
    appId: "1:761637578037:web:a3cb9789fa5d1e87a9d87c"
};

const email = "maintenance@bosconnect.fr";
const password = "maintenance2024!?";

firebase.initializeApp(firebaseConfig);
const database = firebase.database();



function authenticateUser(callback) {
    firebase.auth().signInWithEmailAndPassword(email, password)
        .then(userCredential => {
            const uid = userCredential.user.uid;
            callback(uid);
        })
}

function goToStep(step) {
    const sections = document.querySelectorAll('.section');
    sections.forEach(section => section.classList.remove('visible'));
    document.getElementById(`step${step}`).classList.add('visible');
}

document.getElementById('technicianCheck').addEventListener('change', function () {
    document.getElementById('nextButton').disabled = !this.checked;
});

document.getElementById('testsCompleted').addEventListener('change', function () {
    document.getElementById('exportPdf').disabled = !this.checked;
});

document.getElementById('exportPdf').addEventListener('click', () => {
    const { jsPDF } = window.jspdf;
    const doc = new jsPDF();

    const form = document.getElementById('interventionForm');
    const formData = new FormData(form);

    const interventionName = formData.get('interventionName') || "N/A";
    const interventionNumber = formData.get('interventionNumber') || "N/A";
    const interventionDate = formData.get('interventionDate') || "N/A";

    doc.setFontSize(16);
    doc.text("Fiche de suivi Assistant domotique Bosconnect", doc.internal.pageSize.getWidth() / 2, 20, { align: 'center' });
    doc.setFontSize(12);

    doc.autoTable({
        startY: 40,
        head: [["N° d'intervention", "Date", "Nom intervenant"]],
        body: [
            [interventionNumber, interventionDate, interventionName]
        ],
        theme: 'grid',
        styles: { halign: 'center', fillColor: [0, 0, 0], textColor: [255, 255, 255] },
        headStyles: { fillColor: [0, 0, 0], textColor: [255, 255, 255] },
        bodyStyles: { fillColor: [255, 255, 255], textColor: [0, 0, 0] }
    });

    // Ajout de la liste des outils
    doc.autoTable({
        startY: doc.lastAutoTable.finalY + 20,
        head: [["Outils nécessaires", "Bibliothèques nécessaires"]],
        body: [
            ["Alimentation 12V DC / 1A", "rfid-master"],
            ["Multimètre (voltmètre, ampèremètre, ohmmètre", "TFT_eSPI-master"],
            ["Oscilloscope numérique 2 voies", "BH1750-1.3.0"],
            ["PC avec port USB", ""],
            ["Environnement de développement Arduino", ""],
            ["Capteur de pression", ""],
            ["Capteur de gaz", ""]
        ],
        theme: 'grid',
        styles: { halign: 'center', fillColor: [0, 0, 0], textColor: [255, 255, 255] },
        headStyles: { fillColor: [0, 0, 0], textColor: [255, 255, 255] },
        bodyStyles: { fillColor: [255, 255, 255], textColor: [0, 0, 0] }
    });

    const tableData = [["Test N°", "Description", "Résultat", "Commentaires"]];

    document.querySelectorAll('.test-item').forEach((testItem, index) => {
        const description = testItem.querySelector('label').textContent.trim();
        const result = testItem.querySelector('input[type="radio"]:checked')?.value || "Non renseigné";
        const comments = testItem.querySelector('textarea')?.value.trim() || "Aucun commentaire";

        tableData.push([`Test ${index + 1}`, description, result, comments]);
    });

    doc.autoTable({
        startY: doc.lastAutoTable.finalY + 10,
        head: [tableData[0]],
        body: tableData.slice(1),
        theme: 'grid',
        styles: {
            fontSize: 10,
            halign: 'center',
            fillColor: [0, 0, 0],
            textColor: [255, 255, 255]
        },
        headStyles: { fillColor: [0, 0, 0], textColor: [255, 255, 255] },
        bodyStyles: { fillColor: [255, 255, 255], textColor: [0, 0, 0] },
        columnStyles: {
            0: { cellWidth: 20 },
            1: { cellWidth: 70 },
            2: { cellWidth: 30 },
            3: { cellWidth: 70 },
        }
    });

    doc.autoTable({
        startY: doc.lastAutoTable.finalY + 20,
        head: [["Test N°", "Description", "Résultat", "Commentaires"]],
        body: [
            ["Test 1", "Absence de dégradation mécanique", "", ""],
            ["Test 2", "Présence de tous les composants prévus", "", ""],
            ["Test 3", "Vérification d'absence de tension sur les broches « GND »", "", ""],
            ["Test 4", "Test du capteur de gaz MQ9", "", ""],
            ["Test 5", "Test du capteur de pression BMP280", "", ""],
            ["Test 6", "Test du capteur de température BMP280", "", ""],
            ["Test 7", "Test du capteur de luminosité BH1750", "", ""],
            ["Test 8", "Test du relais double appareil 1", "", ""],
            ["Test 9", "Test du relais double appareil 2", "", ""],
            ["Test 10", "Test du variateur", "", ""]
        ].map(([num, desc]) => {
            const testItem = Array.from(document.querySelectorAll('.test-item')).find(
                item => item.querySelector('label').textContent.includes(num.split(' ')[1])
            );
            const result = testItem?.querySelector('input[type="radio"]:checked')?.value || "Non renseigné";
            const comments = testItem?.querySelector('textarea')?.value.trim() || "Aucun commentaire";
            return [num, desc, result, comments];
        }),
        theme: 'grid',
        styles: {
            fontSize: 10,
            halign: 'center'
        },
        columnStyles: {
            0: { cellWidth: 20 },
            1: { cellWidth: 70 },
            2: { cellWidth: 30 },
            3: { cellWidth: 70 }
        }
    });
    doc.save('Fiche_intervention.pdf');
});

function sendTrueToFirebase() {
    authenticateUser(uid => {
        const ref = database.ref(`utilisateurs/${uid}/testValue`);

        ref.set(true)
            .then(() => {
                ref.once('value').then(snapshot => {
                    document.getElementById('firebaseValue').textContent = snapshot.val();
                });
            })
    });
}

function turnOnDevice() {
    authenticateUser(uid => {
        const deviceRef = database.ref(`utilisateurs/${uid}/deviceStatus`);
        const sensorRef = database.ref(`utilisateurs/${uid}/sensorValue`);

        deviceRef.set(true)
            .then(() => {
                sensorRef.once('value').then(snapshot => {
                    document.getElementById('sensorValue').textContent = snapshot.val().toFixed(2);
                });
            })
    });
}

function turnOffDevice() {
    authenticateUser(uid => {
        const deviceRef = database.ref(`utilisateurs/${uid}/deviceStatus`);
        const sensorRef = database.ref(`utilisateurs/${uid}/sensorValue`);

        deviceRef.set(false)
            .then(() => {
                sensorRef.once('value').then(snapshot => {
                    document.getElementById('sensorValue').textContent = snapshot.val().toFixed(2);
                });
            })
    });
}

function updateSensorValue() {
    authenticateUser(uid => {
        const sensorRef = database.ref(`utilisateurs/${uid}/sensorValue`);
        sensorRef.on('value', snapshot => {
            document.getElementById('sensorValue').textContent = snapshot.val().toFixed(2);
        });
    });
}

updateSensorValue();
