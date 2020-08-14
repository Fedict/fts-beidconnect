//
//  SafariExtensionViewController.h
//  SafariAppExtension Extension
//
//  Created by Vital on 24/06/2020.
//

#import <SafariServices/SafariServices.h>

@interface SafariExtensionViewController : SFSafariExtensionViewController

+ (SafariExtensionViewController *)sharedController;

@end
