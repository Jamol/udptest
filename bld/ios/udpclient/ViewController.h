//
//  ViewController.h
//  udpclient
//
//  Created by Jamol Bao on 7/11/14.
//  Copyright (c) 2014. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController

@property (weak, nonatomic) IBOutlet UITextField *myIP;
@property (weak, nonatomic) IBOutlet UITextField *myPort;
@property (weak, nonatomic) IBOutlet UITextField *peerIP;
@property (weak, nonatomic) IBOutlet UITextField *peerPort;
@property (weak, nonatomic) IBOutlet UITextField *bandwidth;
@property (weak, nonatomic) IBOutlet UITextField *timeSlice;
@property (weak, nonatomic) IBOutlet UIButton *btnStart;
@property (weak, nonatomic) IBOutlet UISwitch *testBurst;
@property (weak, nonatomic) IBOutlet UITextField *burstBytes;

@property (nonatomic) BOOL isRunning;

@end
