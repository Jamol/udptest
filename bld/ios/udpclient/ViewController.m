//
//  ViewController.m
//  udpclient
//
//  Created by Jamol Bao on 7/11/14.
//  Copyright (c) 2014. All rights reserved.
//

#import "ViewController.h"

#include "util.h"
void startTest(const char* bind_addr, uint16_t bind_port, const char* peer_addr, uint16_t peer_port,
               uint32_t bw, uint32_t slice_ms, uint32_t burst_bytes);
void stopTest();
void waitTest();

@interface ViewController ()

@end

@implementation ViewController


- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    char my_ip[64];
    if(km_get_local_ip(my_ip, sizeof(my_ip)) == 0){
        //NSString* s = [NSString stringWithFormat:@"%s" , my_ip];
        my_printf("my ip is %s\n", my_ip);
        [self.myIP setText:@"0.0.0.0"];
    }
    [self.testBurst setOn:NO];
    [self.burstBytes setText:@"30720"];
    [self.burstBytes setEnabled:NO];
    self.isRunning = NO;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)onButtonClicked:(id)sender {
    if(NO == self.isRunning) {
        [self onStart];
    } else {
        [self onStop];
    }
}

- (void) onStart {
    self.isRunning = YES;
    NSString* nsMyIp = self.myIP.text;
    const char* my_ip = [nsMyIp UTF8String];
    NSString* s = self.myPort.text;
    uint16_t my_port = atoi([s UTF8String]);
    NSString* nsPeerIp = self.peerIP.text;
    const char* peer_ip = [nsPeerIp UTF8String];
    s = self.peerPort.text;
    uint16_t peer_port = atoi([s UTF8String]);
    s = self.bandwidth.text;
    uint32_t bw = atoi([s UTF8String]);
    s = self.timeSlice.text;
    uint32_t slice_ms = atoi([s UTF8String]);
    s = self.burstBytes.text;
    uint32_t burst_bytes = atoi([s UTF8String]);
    if(!self.testBurst.on){
        burst_bytes = 0;
    }
    startTest(my_ip, my_port, peer_ip, peer_port, bw, slice_ms, burst_bytes);
    [self.btnStart setTitle:@"Stop" forState:UIControlStateNormal];
}

- (void) onStop {
    stopTest();
    waitTest();
    self.isRunning = NO;
    [self.btnStart setTitle:@"Start" forState:UIControlStateNormal];
    my_printf("test stopped");
}

- (IBAction)onSwitchBurstChanged:(id)sender {
    [self.burstBytes setEnabled:self.testBurst.on];
}

@end
