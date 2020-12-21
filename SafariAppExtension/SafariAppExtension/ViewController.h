//
//  ViewController.h
//  SafariAppExtension
//
//  Created by Vital on 24/06/2020.
//

#import <Cocoa/Cocoa.h>

@interface ViewController : NSViewController

@property (weak, nonatomic) IBOutlet NSTextField * appNameLabel;
@property (weak) IBOutlet NSImageView *appIcon;

- (IBAction)openSafariExtensionPreferences:(id)sender;

@end

