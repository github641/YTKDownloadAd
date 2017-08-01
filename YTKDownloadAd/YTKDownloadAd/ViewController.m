//
//  ViewController.m
//  YTKDownloadAd
//
//  Created by alldk on 2017/8/1.
//  Copyright © 2017年 alldk. All rights reserved.
//

#import "ViewController.h"
#import "HomeAdsVideoRequest.h"
@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event{
    
    HomeAdsVideoRequest *req = [[HomeAdsVideoRequest alloc] initWinthUrl:@"http://renren2.maoyun.tv/ads/jike15s.mp4"];
    
//    req.delegate = self;
//    [req start];
    
    [req startWithCompletionBlockWithSuccess:^(__kindof YTKBaseRequest * _Nonnull request) {
         NSLog(@"%s res:%@", __func__, request.responseData);
        
    } failure:^(__kindof YTKBaseRequest * _Nonnull request) {
        
    }];
    
}

///  Tell the delegate that the request has finished successfully.
///
///  @param request The corresponding request.
- (void)requestFinished:(__kindof YTKBaseRequest *)request{
    NSLog(@"%s", __func__);
}


///  Tell the delegate that the request has failed.
///
///  @param request The corresponding request.
- (void)requestFailed:(__kindof YTKBaseRequest *)request{
    NSLog(@"%s", __func__);
}

@end
