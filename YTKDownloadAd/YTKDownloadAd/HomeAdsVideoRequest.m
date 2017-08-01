//
//  HomeAdsVideoRequest.m
//  YTKDownloadAd
//
//  Created by alldk on 2017/8/1.
//  Copyright © 2017年 alldk. All rights reserved.
//

#import "HomeAdsVideoRequest.h"

@implementation HomeAdsVideoRequest{
    NSString *_url ;
    NSString *_cachePath;
}
- (instancetype)initWinthUrl:(NSString *)url{
    if (self = [super init]) {
        _url = url;
        
        NSString *libPath = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        _cachePath = [libPath stringByAppendingPathComponent:@"AdsVideoCache"];
        
        NSError *error;
        NSFileManager *m = [NSFileManager defaultManager];
        BOOL suc = [m createDirectoryAtPath:_cachePath withIntermediateDirectories:YES attributes:nil error:&error];
        NSLog(@"创建缓存文件夹成功了吗：%@ 沙盒路径：%@", @(suc), _cachePath);
    }
    return self;
}

- (NSString *)requestUrl {
    return _url;
}
- (NSString *)resumableDownloadPath {

     NSString *filePath = [_cachePath stringByAppendingPathComponent:@"1.mp4"];
    return filePath;
}

- (YTKRequestMethod)requestMethod {
    return YTKRequestMethodGET;
}

@end
