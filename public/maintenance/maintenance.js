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
            document.body.classList.add('authenticated');
            callback(uid);
        })
        .catch(error => {
            console.error("Erreur d'authentification:", error);
            alert("Erreur d'authentification. Veuillez réessayer.");
        });
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
    const bosconnectNumber = formData.get('bosconnectNumber') || "N/A"; // Ajout du numéro de carte Bosconnect
    const interventionDate = formData.get('interventionDate') || "N/A";
    const testType = formData.get('testType') || "N/A";

    doc.setFontSize(16);
    doc.setTextColor(40, 40, 40);
    doc.text("Fiche de suivi Assistant domotique Bosconnect", doc.internal.pageSize.getWidth() / 2, 20, { align: 'center' });
    doc.setFontSize(12);

    doc.autoTable({
        startY: 40,
        head: [["Informations du technicien"]],
        body: [
            [`N° d'intervention: ${interventionNumber}`],
            [`Date: ${interventionDate}`],
            [`Nom intervenant: ${interventionName}`],
            [`N° de carte Bosconnect: ${bosconnectNumber}`],
            [`Type de test: ${testType}`]
        ],
        theme: 'plain',
        styles: { halign: 'left', fillColor: [255, 255, 255], textColor: [40, 40, 40] },
        headStyles: { fillColor: [0, 113, 227], textColor: [255, 255, 255], fontSize: 14, fontStyle: 'bold' },
        bodyStyles: { fillColor: [255, 255, 255], textColor: [40, 40, 40], fontSize: 12 }
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
        styles: { halign: 'center', fillColor: [255, 255, 255], textColor: [40, 40, 40] },
        headStyles: { fillColor: [0, 113, 227], textColor: [255, 255, 255] },
        bodyStyles: { fillColor: [245, 245, 245], textColor: [40, 40, 40] }
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
            fillColor: [255, 255, 255],
            textColor: [40, 40, 40]
        },
        headStyles: { fillColor: [0, 113, 227], textColor: [255, 255, 255] },
        bodyStyles: { fillColor: [245, 245, 245], textColor: [40, 40, 40] },
        columnStyles: {
            0: { cellWidth: 20 },
            1: { cellWidth: 70 },
            2: { cellWidth: 30 },
            3: { cellWidth: 70 },
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

function downloadFiles() {
    const zipUrl = '../assets/bosconnect_test_files.zip';
    const link = document.createElement('a');
    link.href = zipUrl;
    link.download = 'bosconnect_test_files.zip';
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}
