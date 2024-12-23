function goToStep(step) {
    const sections = document.querySelectorAll('.section');
    sections.forEach(section => section.classList.remove('visible'));
    document.getElementById(`step${step}`).classList.add('visible');
}

document.getElementById('technicianCheck').addEventListener('change', function () {
    document.getElementById('nextButton').disabled = !this.checked;
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
            ["Multimètre (voltmètre, ampèremètre, ohmmètre)", "TFT_eSPI-master"],
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

    doc.save('Fiche_intervention_formalisee.pdf');
});
