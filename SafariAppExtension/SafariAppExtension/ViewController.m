//
//  ViewController.m
//  SafariAppExtension
//
//  Created by Vital on 24/06/2020.
//

#import "ViewController.h"
#import <SafariServices/SFSafariApplication.h>

@implementation ViewController



- (void)viewDidLoad {
    [super viewDidLoad];
    self.appNameLabel.stringValue = @"BOSA eID App Extension for Safari";
   
   NSString *path = [[NSBundle mainBundle] pathForResource:@"BOSA logo" ofType:@"png"];
   NSData *myData = [NSData dataWithContentsOfFile:path];
   [self.appIcon setImage:[[NSImage alloc ] initWithData:myData]];
}

- (IBAction)openSafariExtensionPreferences:(id)sender {
    [SFSafariApplication showPreferencesForExtensionWithIdentifier:@"be.bosa.eidapp-extension" completionHandler:^(NSError * _Nullable error) {
        if (error) {
           NSLog(@"%d, %@", (int) error.code, error.localizedDescription);
               dispatch_time_t delayTime = dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC);
               dispatch_after(delayTime, dispatch_get_main_queue(), ^(void){
                             NSAlert *alert = [[NSAlert alloc] init];
                             [alert addButtonWithTitle:@"Close"];
                             [alert setMessageText:@"Could not open Safari Preferences"/*error.localizedDescription*/];
                              NSString *text = [NSString stringWithFormat:@"Please try installing again.\n (%@)", error.localizedDescription];
                             [alert setInformativeText:text];
                             [alert setAlertStyle:NSWarningAlertStyle];
                             if ([alert runModal] == NSAlertFirstButtonReturn) {
                             }
               });
        }
    }];
   
   dispatch_time_t delayTime = dispatch_time(DISPATCH_TIME_NOW, 3 * NSEC_PER_SEC);
   dispatch_after(delayTime, dispatch_get_main_queue(), ^(void){
      exit(0);
   });
}

@end
