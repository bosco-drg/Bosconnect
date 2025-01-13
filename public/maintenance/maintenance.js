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

    const interventionName = formData.get('interventionName') || "";
    const interventionNumber = formData.get('interventionNumber') || "";
    const bosconnectNumber = formData.get('bosconnectNumber') || "";
    const interventionDate = formData.get('interventionDate') || "";
    const testType = formData.get('testType') || "";

    doc.setFontSize(20);
    doc.setTextColor(40, 40, 40);
    doc.setFont("helvetica", "bold");
    doc.text("Fiche de Suivi Assistant Domotique Bosconnect", doc.internal.pageSize.getWidth() / 2, 20, { align: 'center' });
    doc.setFontSize(12);
    doc.setFont("helvetica", "normal");

    doc.autoTable({
        startY: 40,
        head: [["Informations du technicien"]],
        body: [
            [{ content: `N° d'intervention: ${interventionNumber}`, styles: { fontStyle: 'bold' } }],
            [{ content: `Date: ${interventionDate}`, styles: { fontStyle: 'bold' } }],
            [{ content: `Nom intervenant: ${interventionName}`, styles: { fontStyle: 'bold' } }],
            [{ content: `N° de carte Bosconnect: ${bosconnectNumber}`, styles: { fontStyle: 'bold' } }],
            [{ content: `Type de test: ${testType}`, styles: { fontStyle: 'bold' } }]
        ],
        theme: 'plain',
        styles: { halign: 'left', fillColor: [255, 255, 255], textColor: [40, 40, 40] },
        headStyles: { fillColor: [0, 113, 227], textColor: [255, 255, 255], fontSize: 10, fontStyle: 'bold' },
        bodyStyles: { fillColor: [255, 255, 255], textColor: [40, 40, 40], fontSize: 10 }
    });

    const tableData = [["Test N°", "Description", "Résultat", "Commentaires"]];

    let testIndex = 1;
    document.querySelectorAll('.test-item').forEach((testItem) => {
        if (!testItem.querySelector('#finalComments')) {
            const labelElement = testItem.querySelector('label');
            const description = labelElement ? labelElement.textContent.trim() : `Test ${testIndex}`;
            const radioChecked = testItem.querySelector('input[type="radio"]:checked');
            const result = radioChecked ? radioChecked.value : " ";
            const textArea = testItem.querySelector('textarea');
            const comments = textArea ? textArea.value.trim() : "";

            tableData.push([`${testIndex}`, description, result, comments]);
            testIndex++;
        }
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

    const finalComments = document.getElementById('finalComments').value || " ";
    
    doc.autoTable({
        startY: doc.lastAutoTable.finalY + 10,
        head: [["Commentaires généraux de l'intervention"]],
        body: [[finalComments]],
        theme: 'grid',
        styles: {
            fontSize: 10,
            halign: 'left',
            fillColor: [255, 255, 255],
            textColor: [40, 40, 40]
        },
        headStyles: { 
            fillColor: [0, 113, 227], 
            textColor: [255, 255, 255],
            halign: 'center'
        },
        columnStyles: {
            0: { cellWidth: 190 }
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
