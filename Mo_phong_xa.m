% Thông số pack pin 3S
V_cell_max = 4.2;         % Điện áp tối đa của mỗi cell (V)
V_cell_min = 3.0;         % Điện áp tối thiểu của mỗi cell (V)
V_pack_max = V_cell_max * 3; % Điện áp tối đa của pack 3S (12.6V)
V_pack_min = V_cell_min * 3; % Điện áp tối thiểu của pack 3S (9.0V)
I_discharge = 0.5;        % Dòng xả cố định (A)
C_bat = 2.0;              % Dung lượng pin (Ah)
SOC_initial = 1.0;        % Trạng thái sạc ban đầu (SOC), 100% (đầy)
T_initial = 25;           % Nhiệt độ ban đầu (độ C)
T_increase_discharge = 10; % Giả sử nhiệt độ TĂNG thêm khi xả (thường xả pin sẽ nóng lên)

% Thời gian mô phỏng
t_total = 60 * 60;        % Tổng thời gian mô phỏng (giây), 60 phút
dt = 1;                   % Bước thời gian (giây)
time = 0:dt:t_total;

% Khởi tạo biến
V_pack = zeros(size(time));
I_discharge_array = zeros(size(time));
SOC = zeros(size(time));
Temperature = zeros(size(time));
SOC(1) = SOC_initial;
Temperature(1) = T_initial;

% Mô phỏng quá trình xả của pack 3S
for k = 2:length(time)
    % Tính điện áp pack dựa vào SOC (tuyến tính đơn giản)
    V_pack(k-1) = V_pack_min + (V_pack_max - V_pack_min) * SOC(k-1);
    
    % Kiểm tra điều kiện xả
    if V_pack(k-1) > V_pack_min && SOC(k-1) > 0
        I_discharge_array(k) = -I_discharge; % Dòng xả (quy ước âm)
    else
        I_discharge_array(k) = 0; % Dừng xả khi chạm ngưỡng điện áp thấp
    end
    
    % Cập nhật SOC
    SOC(k) = SOC(k-1) + (I_discharge_array(k) * dt) / (C_bat * 3600);
    SOC(k) = max(SOC(k), 0);  % Giới hạn SOC tối thiểu là 0%
    
    % Cập nhật nhiệt độ (Khi xả pin thường nóng lên)
    if I_discharge_array(k) < 0
        Temperature(k) = Temperature(k-1) + (T_increase_discharge / t_total) * dt;
    else
        Temperature(k) = Temperature(k-1); 
    end
end
V_pack(end) = V_pack_min + (V_pack_max - V_pack_min) * SOC(end);

% Vẽ đồ thị
figure;

% Đồ thị Điện áp
subplot(4,1,1);
plot(time/60, V_pack, 'r', 'LineWidth', 1.5);
xlabel('Thời gian (phút)');
ylabel('Điện áp (V)');
title('Biểu đồ Điện áp Pack 3S');
grid on;
ylim([V_pack_min-1 V_pack_max+1]);

% Đồ thị Dòng điện
subplot(4,1,2);
plot(time/60, I_discharge_array, 'b', 'LineWidth', 1.5);
xlabel('Thời gian (phút)');
ylabel('Dòng xả (A)');
title('Dòng điện xả');
grid on;
ylim([-1 0.5]);

% Đồ thị SOC
subplot(4,1,3);
plot(time/60, SOC * 100, 'g', 'LineWidth', 1.5);
xlabel('Thời gian (phút)');
ylabel('SOC (%)');
title('Trạng thái sạc (SOC)');
grid on;
ylim([0 110]);

% Đồ thị Nhiệt độ
subplot(4,1,4);
plot(time/60, Temperature, 'Color', [1 0.5 0], 'LineWidth', 1.5);
xlabel('Thời gian (phút)');
ylabel('Nhiệt độ (°C)');
title('Nhiệt độ pin khi xả');
grid on;
