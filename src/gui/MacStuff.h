#ifndef MACSTUFF_H
#define MACSTUFF_H
#include <QObject>
#ifdef Q_OS_MAC

#import <AppKit/NSButton.h>
#import <AppKit/NSToolbar.h>
#import <AppKit/NSToolbarItem.h>
#import "MacGlue.h"
#import "MainWindow.h"


@interface MacStuff : NSObject <NSToolbarDelegate>
{
@private
	MainWindow *_mainWindow;
}
- (NSArray<NSString*> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar;
- (NSArray<NSString*> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar;

- (NSToolbarItem *) toolbar:(NSToolbar *)toolbar
		itemForItemIdentifier:(NSString *)dataString
  willBeInsertedIntoToolbar:(BOOL)flag;
- (NSButton*)addButton:(NSString*)dataString;
- (void)doStuff:(NSButton *)sender;
@end
#else
class MacStuff {
};
#endif
#endif // MACSTUFF_H
