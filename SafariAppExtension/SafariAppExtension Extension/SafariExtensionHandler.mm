//
//  SafariExtensionHandler.m
//  SafariAppExtension Extension
//
//  Created by Vital on 24/06/2020.
//

#import "SafariExtensionHandler.h"
#import "SafariExtensionViewController.h"

#include "log.hpp"
#include "RequestHandler.hpp"
#include <iostream>
#include <sstream>
#include "util.h"
#include "general.h"

@interface SafariExtensionHandler ()

@end
   
@implementation SafariExtensionHandler

using namespace std;

- (void)messageReceivedWithName:(NSString *)messageName fromPage:(SFSafariPage *)page userInfo:(NSDictionary *)userInfo {

   // This method will be called when a content script provided by your extension calls safari.extension.dispatchMessage("message").
   if ([messageName isEqualToString: @"ping"]) {
       return;
    }
    [page getPagePropertiesWithCompletionHandler:^(SFSafariPageProperties *properties) {
        NSLog(@"The extension received a message (%@) from a script injected into (%@) with userInfo (%@)", messageName, properties.url, userInfo);
    }];
   
   NSError * _Nullable error;
   NSData* jsonData = [NSJSONSerialization dataWithJSONObject:userInfo options:kNilOptions error:&error];
   
   NSString *strData = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
   shared_ptr<stringstream> ssRequest = std::make_shared<stringstream>([strData UTF8String]);
     
   shared_ptr<RequestHandler> handler = RequestHandler::createRequestHandler(ssRequest);
   string ssResponse = handler->process();

   log_info(ssResponse.c_str());
   
   NSString *str = [[NSString alloc]initWithUTF8String:ssResponse.c_str()];
   NSData *data = [str dataUsingEncoding:NSUTF8StringEncoding];
   
   NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:data options:kNilOptions error:&error];
   if (error) {
           // Insert code to inform the user something went wrong.
   }

   NSMutableDictionary *dd = [[NSMutableDictionary alloc]initWithDictionary:dict];
   [dd setValue:@"background" forKey:@"src"];
   [dd setValue:@"safari" forKey:@"extensionBrowser"];
   [dd setValue:@"1.0.1" forKey:@"extensionVersion"];

   [page dispatchMessageToScriptWithName:@"message_fromapp" userInfo:dd];
}

//- (void)toolbarItemClickedInWindow:(SFSafariWindow *)window {
//    // This method will be called when your toolbar item is clicked.
//    NSLog(@"The extension's toolbar item was clicked");
//}

- (void)validateToolbarItemInWindow:(SFSafariWindow *)window validationHandler:(void (^)(BOOL enabled, NSString *badgeText))validationHandler {
    // This method will be called whenever some state changes in the passed in window. You should use this as a chance to enable or disable your toolbar item and set badge text.
    validationHandler(YES, nil);
}

- (SFSafariExtensionViewController *)popoverViewController {
    return [SafariExtensionViewController sharedController];
}

@end
