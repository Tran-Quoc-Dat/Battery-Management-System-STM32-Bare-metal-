% Thông số pack pin 2S
V_cell_max = 4.2;         % Điện áp tối đa của mỗi cell (V)
V_pack_max = V_cell_max * 2; % Điện áp tối đa của pack 2S (V)
I_max = 1.04;             % Dòng sạc tối đa (A)
C_bat = 2;                % Dung lượng pin (Ah)
SOC_initial = 0.2;        % Trạng thái sạc ban đầu (SOC)
T_initial = 25;           % Nhiệt độ ban đầu (độ C)
T_max_increase = 15;      % Tăng nhiệt độ tối đa trong quá trình sạc (giả sử đơn giản)

% Giới hạn SOC khi đạt điện áp tối đa (trạng thái trước giai đoạn CV)
SOC_at_Vmax = 0.8;

% Thời gian mô phỏng
t_total = 2 * 3600;       % Tổng thời gian mô phỏng (giây)
dt = 1;                   % Bước thời gian (giây)
time = 0:dt:t_total;

% Khởi tạo biến
V_pack = zeros(size(time));
I_charge = zeros(size(time));
SOC = zeros(size(time));
Temperature = zeros(size(time));
SOC(1) = SOC_initial;
Temperature(1) = T_initial;

% Sạc CC-CV cho pack 2S
for k = 2:length(time)
    % Tính điện áp pack dựa vào SOC (đơn giản hóa tuyến tính)
    V_pack(k) = V_pack_max * min(SOC(k-1), SOC_at_Vmax);
    
    % Giai đoạn sạc CC
    if SOC(k-1) < SOC_at_Vmax
        I_charge(k) = I_max;
    else
        % Giai đoạn sạc CV: Dòng sạc giảm dần
        I_charge(k) = max(0, I_max * (1 - (SOC(k-1) - SOC_at_Vmax) / (1 - SOC_at_Vmax)));
    end
    
    % Cập nhật SOC
    SOC(k) = SOC(k-1) + (I_charge(k) * dt) / (C_bat * 3600);
    SOC(k) = min(SOC(k), 1);  % Giới hạn SOC tối đa là 100%
    
    % Cập nhật nhiệt độ (giả sử nhiệt độ tăng theo dòng sạc)
    if I_charge(k) > 0
        Temperature(k) = Temperature(k-1) + (T_max_increase / t_total) * dt;
    else
        % Nhiệt độ giảm dần khi sạc xong
        Temperature(k) = max(T_initial, Temperature(k-1) - (T_max_increase / t_total) * dt);
    end
end

% Vẽ đồ thị
figure;

% Đồ thị Voltage
subplot(4,1,1);
plot(time/3600, V_pack, 'r', 'LineWidth', 1.5);  % Đồ thị điện áp màu đỏ
xlabel('Thời gian (giờ)');
ylabel('Điện áp (V)');
title('Biểu đồ Điện áp (Voltage)');
grid on;
ylim([0 9]);                      
yticks(0:1:9);                    

% Đồ thị Current
subplot(4,1,2);
plot(time/3600, I_charge, 'b', 'LineWidth', 1.5);  % Đồ thị dòng sạc màu xanh
xlabel('Thời gian (giờ)');
ylabel('Dòng sạc (A)');
title('Biểu đồ Dòng điện (Current)');
grid on;
ylim([0 1.2]);                    
yticks(0:0.2:1.2);                

% Đồ thị SOC
subplot(4,1,3);
plot(time/3600, SOC * 100, 'g', 'LineWidth', 1.5);  % Đồ thị SOC màu xanh lá
xlabel('Thời gian (giờ)');
ylabel('Mức sạc SOC (%)');
title('Trạng thái sạc (SOC)');
grid on;
yticks(0:20:100);                 

% Đồ thị Temperature
subplot(4,1,4);
plot(time/3600, Temperature, 'Color', [1 0.5 0], 'LineWidth', 1.5);  % Đồ thị nhiệt độ màu cam
xlabel('Thời gian (giờ)');
ylabel('Nhiệt độ (°C)');
title('Biểu đồ Nhiệt độ');
grid on;
ylim([0 40]);                     
yticks(0:10:40); 
