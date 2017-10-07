#import "MacStuff.h"

#include <QtMacExtras>
#include <QToolBar>
#include <QAction>
#include <AppKit/NSToolbar.h>
#include <AppKit/NSToolbarItem.h>
#include <AppKit/NSButton.h>
#include <AppKit/NSImage.h>

#include "MainWindow.h"
#include <QMetaObject>
#include <QIcon>
#include <QLayout>

/*
 * "[...]Objective-C is a fundamentally simple language.
 * Its syntax is small, unambiguous, and easy to learn." –
 * https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/OOP_ObjC/Articles/ooWhy.html
 *
 * ^ Apple's attempt at humor.
 */
@implementation MacStuff {

}
- (NSButton *)addButton:(NSString *)dataString {
		// Create a button with the dimensions 40x22
		NSButton *button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 40, 22)];

		// Set the id to the data string.
		[button setIdentifier:dataString];

		// Parse the id, which serves as the data for the button. Separated by '%'.
		// title%icon_path&function_to_call%isToggleable
		NSArray *data = [dataString componentsSeparatedByString:@"%"];

		// The title of the item.
		NSString *title = [data objectAtIndex:0];

		// Generating the icon. Broken into steps.
		// Convert the path from an NSString to a QString
		QString path = QString::fromNSString([data objectAtIndex:1]);
		// Create a QIcon from that path.
		QIcon qIcon(path);
		// Convert to a 16x16 QPixmap.
		QPixmap pixmap = qIcon.pixmap(QSize(16, 16));
		// Convert to an NSImage to set as the icon.
		NSImage *icon = QtMac::toNSImage(pixmap);
		// Set the button's icon.
		[button setImage:icon];

		// Check if this is a toggle switch or a regular button
		bool isToggleable = [[data objectAtIndex:3] isEqualTo:@"1"];

		// Set the name of the button.
		//[button setTitle:title];

		if (isToggleable) {
			// A toggle button
			[button setButtonType:NSOnOffButton];
			// Flag that it is toggleable, because for some stupid reason, there is no buttonType option.
			[button setTag:1];
		} else {
			// A standard button.
			[button setButtonType:NSMomentaryPushInButton];
			// Not toggleable
			[button setTag:0];
		}

		// Fancy white mac buttons.
		[button setBezelStyle:NSTexturedRoundedBezelStyle];

		// Set the target to this and sets the action to doStuff.
		// basically connect(button, SIGNAL(triggered()), this, SLOT(doStuff()));
		[button setTarget:self];
		[button setAction:@selector(doStuff:)];

		return button;
}
- (NSArray<NSString*> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
	//QList<QString> *list = _mainWindow->toolbarActions();
	NSMutableArray *array = [[NSMutableArray alloc] init];
	foreach(QString *string, *(_mainWindow->toolbarActions())) {
		[array addObject:string->toNSString()];
	}
	return array;
}
- (NSArray<NSString*> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
	// TODO: implement properly
	return [self toolbarAllowedItemIdentifiers:toolbar];
}

- (NSToolbarItem *) toolbar:(NSToolbar *)toolbar
	 itemForItemIdentifier:(NSString *)dataString
	 willBeInsertedIntoToolbar:(BOOL)flag {

	NSButton *button = [[self addButton:dataString] autorelease];
	NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier:dataString]
																 autorelease];
	// set the title
	NSString *title = [[dataString componentsSeparatedByString:@"%"] objectAtIndex:0];
	[item setLabel:title];
	[item setToolTip:title];

	[item setView:button];

	return item;
}
- (void)doStuff: (NSButton *)sender {
	NSString *dataString = [sender identifier];
	NSArray<NSString *> *dataArray = [dataString componentsSeparatedByString:@"%"];
	if ([sender tag] == 0) {
		QMetaObject::invokeMethod(_mainWindow,
									  const_cast<const char *>([[dataArray objectAtIndex:2] cStringUsingEncoding:[NSString
															 defaultCStringEncoding]]));
	} else {
		QMetaObject::invokeMethod(_mainWindow,
										  const_cast<const char *>([[dataArray objectAtIndex:2] cStringUsingEncoding:[NSString
																 defaultCStringEncoding]]), Q_ARG(bool, ([sender state] == NSOnState)));
	}
}
- (id)initWithMainWindow:(MainWindow *)window {
	self = [super init];
	if (self) {
		_mainWindow = window;
	}
	return self;
}
QMacToolBar *setupMacActions(MainWindow *window) {

	MacStuff *stuff = [[MacStuff alloc] initWithMainWindow:window];
	QMacToolBar *toolBar = new QMacToolBar(window);

	NSToolbar *nativeToolBar = toolBar->nativeToolbar();
	[nativeToolBar setDelegate:stuff];
	return toolBar;
}
@end
