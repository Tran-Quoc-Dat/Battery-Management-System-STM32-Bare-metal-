% Thông s? pack pin 3S
V_cell_max = 4.2;         % ?i?n áp t?i ?a c?a m?i cell (V)
V_cell_min = 3.0;         % ?i?n áp t?i thi?u c?a m?i cell (V)
V_pack_max = V_cell_max * 3; % ?i?n áp t?i ?a c?a pack 3S (12.6V)
V_pack_min = V_cell_min * 3; % ?i?n áp t?i thi?u c?a pack 3S (9.0V)
I_discharge = 0.5;        % Dòng x? c? ??nh (A)
C_bat = 2.0;              % Dung l??ng pin (Ah)
SOC_initial = 1.0;        % Tr?ng thái s?c ban ??u (SOC), 100% (??y)
T_initial = 25;           % Nhi?t ?? ban ??u (?? C)
T_increase_discharge = 10; % Gi? s? nhi?t ?? T?NG thêm khi x? (th??ng x? pin s? nóng lên)

% Th?i gian mô ph?ng
t_total = 60 * 60;        % T?ng th?i gian mô ph?ng (giây), 60 phút
dt = 1;                   % B??c th?i gian (giây)
time = 0:dt:t_total;

% Kh?i t?o bi?n
V_pack = zeros(size(time));
I_discharge_array = zeros(size(time));
SOC = zeros(size(time));
Temperature = zeros(size(time));
SOC(1) = SOC_initial;
Temperature(1) = T_initial;

% Mô ph?ng quá trình x? c?a pack 3S
for k = 2:length(time)
    % Tính ?i?n áp pack d?a vào SOC (tuy?n tính ??n gi?n)
    V_pack(k-1) = V_pack_min + (V_pack_max - V_pack_min) * SOC(k-1);
    
    % Ki?m tra ?i?u ki?n x?
    if V_pack(k-1) > V_pack_min && SOC(k-1) > 0
        I_discharge_array(k) = -I_discharge; % Dòng x? (quy ??c âm)
    else
        I_discharge_array(k) = 0; % D?ng x? khi ch?m ng??ng ?i?n áp th?p
    end
    
    % C?p nh?t SOC
    SOC(k) = SOC(k-1) + (I_discharge_array(k) * dt) / (C_bat * 3600);
    SOC(k) = max(SOC(k), 0);  % Gi?i h?n SOC t?i thi?u là 0%
    
    % C?p nh?t nhi?t ?? (Khi x? pin th??ng nóng lên)
    if I_discharge_array(k) < 0
        Temperature(k) = Temperature(k-1) + (T_increase_discharge / t_total) * dt;
    else
        Temperature(k) = Temperature(k-1); 
    end
end
V_pack(end) = V_pack_min + (V_pack_max - V_pack_min) * SOC(end);

% V? ?? th?
figure;

% ?? th? ?i?n áp
subplot(4,1,1);
plot(time/60, V_pack, 'r', 'LineWidth', 1.5);
xlabel('Th?i gian (phút)');
ylabel('?i?n áp (V)');
title('Bi?u ?? ?i?n áp Pack 3S');
grid on;
ylim([V_pack_min-1 V_pack_max+1]);

% ?? th? Dòng ?i?n
subplot(4,1,2);
plot(time/60, I_discharge_array, 'b', 'LineWidth', 1.5);
xlabel('Th?i gian (phút)');
ylabel('Dòng x? (A)');
title('Dòng ?i?n x?');
grid on;
ylim([-1 0.5]);

% ?? th? SOC
subplot(4,1,3);
plot(time/60, SOC * 100, 'g', 'LineWidth', 1.5);
xlabel('Th?i gian (phút)');
ylabel('SOC (%)');
title('Tr?ng thái s?c (SOC)');
grid on;
ylim([0 110]);

% ?? th? Nhi?t ??
subplot(4,1,4);
plot(time/60, Temperature, 'Color', [1 0.5 0], 'LineWidth', 1.5);
xlabel('Th?i gian (phút)');
ylabel('Nhi?t ?? (°C)');
title('Nhi?t ?? pin khi x?');
grid on;