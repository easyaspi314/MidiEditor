// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "MidFix4AgbDialog.h"
#include "MidiFixer.h"
#include "../protocol/Protocol.h"

MidFix4AgbDialog::MidFix4AgbDialog(MidiFile *f, QWidget *parent) : QDialog(parent)
{

    file = f;
    setWindowTitle(tr("Optimize MIDI for GBA"));
    layout = new QFormLayout(this);

    layout->addRow(new QLabel(tr("midfix4agb (c) 2014 by ipatix"), this));

    modScale = new QDoubleSpinBox(this);
    modScale->setDecimals(2);
    modScale->setValue(0.5);
    modScale->setMinimum(0.01);
    layout->addRow(tr("Modulation scaling:"), modScale);

    modType = new QComboBox(this);
    modType->addItem(tr("Pitch"));
    modType->addItem(tr("Volume"));
    modType->addItem(tr("Panning"));
    layout->addRow(tr("Modulation type"), modType);

    layout->addRow(separator());

    fixVolScale = new QComboBox(this);
    fixVolScale->addItem(tr("Enabled"));
    fixVolScale->addItem(tr("Combine with expression only"));
    fixVolScale->addItem(tr("Disabled"));
    layout->addRow(tr("Volume scaling:"), fixVolScale);

    //fixLoop = new QCheckBox(tr("Fix looping"), this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &MidFix4AgbDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MidFix4AgbDialog::reject);
    layout->addRow(buttonBox);

    setLayout(layout);
    //show();
}

void MidFix4AgbDialog::accept() {
    if (!file)
        return;

    file->protocol()->startNewAction(tr("Optimized MIDI for GBA"));
    QProgressDialog progress(tr("Optimizing MIDI…"), tr("Cancel"), 0, 5, this);
    progress.setWindowModality(Qt::WindowModal);

    MidiFixer::addAgbCompatibleEvents(file, modType->currentIndex());
    progress.setValue(1);
    int fixVolScaleVal = fixVolScale->currentIndex();
    if (fixVolScaleVal < 2)
        MidiFixer::combineVolumeAndExpression(file);
    progress.setValue(2);
    MidiFixer::addModulationScale(file, modScale->value());
    progress.setValue(3);
    if (fixVolScaleVal == 0)
        MidiFixer::addExponentialScale(file);
    progress.setValue(5);
    file->protocol()->endAction();

    hide();
}

QWidget *MidFix4AgbDialog::separator() {
    QFrame *f0 = new QFrame(this);
    f0->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    return f0;
}
