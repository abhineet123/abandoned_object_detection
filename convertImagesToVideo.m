function [] = convertImagesToVideo(template,no_of_frames,frame_rate,output_file)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
outputVideo = VideoWriter(output_file);
outputVideo.FrameRate = frame_rate;
open(outputVideo);
for frame_no=0:no_of_frames
    file_name=sprintf('%s%05d.jpeg',template,frame_no);
    img = imread(file_name);
    writeVideo(outputVideo,img);
    fprintf('%d\n',frame_no);
end
close(outputVideo);
end

