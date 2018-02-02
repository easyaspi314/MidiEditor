#ifndef MIDFIX4AGBDIALOG_H
#define MIDFIX4AGBDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QProgressDialog>
#include "../midi/MidiFile.h"

class MidFix4AgbDialog : public QDialog
{
		Q_OBJECT
	public:
		explicit MidFix4AgbDialog(MidiFile *f, QWidget *parent = nullptr);

	signals:

	public slots:
		void accept();

	private:
		MidiFile *file;
		QFormLayout *layout;
		QDoubleSpinBox *modScale;
		QComboBox *modType;
		QComboBox *fixVolScale;
		QWidget *separator();
		QProgressDialog *progressDialog;

};

#endif // MIDFIX4AGBDIALOG_H
